/**************************************************************
* Class:  CSC-415-0# Spring 2021
* Name: Zhuozhuo Liu
* Student ID: 921410045
* GitHub UserID: liuzz10
* Project: Assignment 5 – Buffered I/O
*
* File: b_io.c
*
* Description: Buffered io module - Now with b_write. 
*				This file include 3 main function - b_open, b_read, and b_write
*				- that are used to read data and write by the main file.
*
**************************************************************/

#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>			// for malloc
#include <string.h>			// for memcpy
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include "b_io.h"
#include <stdbool.h>		// for write buffer indicator
#include "fsLow.h"  		// import MINBLOCKSIZE 512
#include "mfs.h"
#include <math.h>			// for ceil()
#include <stdlib.h>
#include "freeSpace.h"

#define MAXFCBS 20
#define BUFSIZE 512
#define GETMOREBLOCKS 10

typedef struct b_fcb
	{
	char * fileName;	// file name
	int fcbStatus;		// -1 means available, 1 means not available
	int readWriteFlags;	// RDONLY, WRONLY, RW
	uint64_t startingLBA;	//the LBA of the first block
	char * buf;		//holds the open file buffer
	int index;		//holds the current position in the buffer
	int buflen;		//holds how many valid bytes are in the buffer
	bool writeBufferNonEmpty;	// track if we need to flush write buffer
	int cursorInDisk;	// the cursor that tracks which block we are on disk
	int bytesRead;	// the cursor that tracks # bytes read (in case it's reaching EOF)
	off_t fileSize;	// the current size of the file
	blkcnt_t fileBlocksAllocated;	// total number of the blocks that the file allocated
	} b_fcb;
	
b_fcb fcbArray[MAXFCBS];

int startup = 0;	//Indicates that this has not been initialized

char write_Buffer[BUFSIZE]; // Stack Allocated Array and Counter

//Method to initialize our file system
void b_init ()
{
	//init fcbArray to all free
	for (int i = 0; i < MAXFCBS; i++)
		{
		fcbArray[i].fcbStatus = -1; //indicates a free fcbArray
		}
		
	startup = 1;
}

//Method to get a free FCB element, intialize fcbStatus
int b_getFCB ()
{
	for (int i = 0; i < MAXFCBS; i++)
	{
		if (fcbArray[i].fcbStatus == -1)
		{
			fcbArray[i].fcbStatus = 1; // used but not assigned
			return i;		//Not thread safe
		}
	}
	return -1;  //all in use
}
	

// Interface to open a buffered file
int b_open (char * filename, int flags)
{
	int returnFd;
	if (startup == 0) b_init();  //Initialize our system
	
	// check if all 20 spots are used at the same time
	returnFd = b_getFCB();				// find an available spot, get our own file descriptor
	if (returnFd  == -1) {
		return -1;						// no spot available, all 20 files are used, not able to open
	}				
	
	// allocate our buffer
	fcbArray[returnFd].buf = malloc (BUFSIZE);
	if (fcbArray[returnFd].buf  == NULL)
	{
		// very bad, we can not allocate our buffer
		fcbArray[returnFd].fcbStatus = -1; 	// free FCB
		return -1;
	}
	
	// initialize other fcb properties
	fcbArray[returnFd].fileName = filename; // save the filename
	fcbArray[returnFd].buflen = 0; 			// have not read anything yet
	fcbArray[returnFd].index = 0;			// have not read anything yet
	fcbArray[returnFd].writeBufferNonEmpty = false;	// track if we need to flush write buffer
	fcbArray[returnFd].cursorInDisk = 0;	// the cursor that tracks which block we are on disk
	fcbArray[returnFd].bytesRead = 0;

	// Get the starting LBA, blocks allocated, file size of the file by passing filename, O_CREAT/O_TRUNC flag to directory fucntion.
	fcbArray[returnFd].startingLBA = getFileLBA(filename, flags); // If the file existing, return the starting LBA of the existing file. If not, it will create a file and retur its starting LBA.
	fcbArray[returnFd].fileBlocksAllocated = getBlocks(filename);
	fcbArray[returnFd].fileSize = getFileSize(filename);

	// O_RDWR
	if ((flags & O_ACCMODE) == O_RDWR) {
		fcbArray[returnFd].readWriteFlags = O_RDWR;
	
	// O_RDONLY
	} else if ((flags & O_ACCMODE) == O_RDONLY) {
		fcbArray[returnFd].readWriteFlags = O_RDONLY;
	
	// O_WRONLY
	} else if ((flags & O_ACCMODE) == O_WRONLY) {
		fcbArray[returnFd].readWriteFlags = O_WRONLY;
	} else {
		fcbArray[returnFd].readWriteFlags = -1;	// -1 means not initialized
	}

	return (returnFd);
}


// Interface to write a buffer	
int b_write (int fd, char * buffer, int count)
	{
	int bytesWrite;				// how many bytes written to the file, there can be some left in the buffer
	int bytesNeededToFull;		// how many bytes are left in my buffer to fill up the buffer
	int pointer;				// how many bytes have been processed in the call's buffer
	int loadToBuffer;			// how many bytes is going to be load to buffer
	int fileUsedBlocks;			// how many blocks the file has takne in the disk (this is <= blocks allocated)

	if (startup == 0) b_init();  //Initialize our system

	// check that fd is between 0 and (MAXFCBS-1)
	if ((fd < 0) || (fd >= MAXFCBS))
		{
		return -1; 					//invalid file descriptor
		}
		
	if (fcbArray[fd].fcbStatus == -1)		//File not open for this descriptor
		{
		return -1;
		}	
	
	// If the file is not for write, stop and return error
	if (fcbArray[fd].readWriteFlags != O_WRONLY && fcbArray[fd].readWriteFlags != O_RDWR) {
		printf("ERROR: No access to write.\n");
		return -1;
	}

	// Initialize the pointer pointing to the first byte in caller's buffer
	pointer = 0;
	
	// Convert fileSize to how many blocks the file is taking (this is not the total blocks allocated)
	fileUsedBlocks = ceil(fcbArray[fd].fileSize / 512);

	// Writing to the file from caller's buffer until reaching the end
	while (count > 0) {
		// number of bytes needed to fill the buffer
		bytesNeededToFull = BUFSIZE - fcbArray[fd].buflen;
		
		// get the number of bytes going to be load to the buffer from caller's buffer
		// it's min(count, bytesNeededToFull)
		if (count < bytesNeededToFull) {
			loadToBuffer = count;		// when the buffer will not be filled up
		} else {
			loadToBuffer = bytesNeededToFull;		// when the buffer will be filled up, then we can write
		}

		// load bytes to buffer from caller's buffer
		memcpy (fcbArray[fd].buf + fcbArray[fd].buflen, buffer + pointer, loadToBuffer);

		// update pointer - skipping those we have loaded and pointing to the next byte to load
		pointer += loadToBuffer;

		// update total remaining bytes need to load
		count -= loadToBuffer;

		// update existing bytes in the buffer
		fcbArray[fd].buflen += loadToBuffer;

		// write bytes from buffer to the file ONLY when the buffer is full
		if (fcbArray[fd].buflen == BUFSIZE) {

			// check if all allocated blocks have been used
			// if yes, call free space manager to get 10 more blocks
			if (fileUsedBlocks == fcbArray[fd].fileBlocksAllocated) { 
				u_int64_t res = expandFreeSection(fcbArray[fd].startingLBA, fcbArray[fd].fileBlocksAllocated, fcbArray[fd].fileBlocksAllocated + GETMOREBLOCKS);
				if (res == 0) {
					printf("ERROR: Write failed");
					return -1;
				}

				// Update LBA and blocks allocated in fcb
				fcbArray[fd].startingLBA = res;
				fcbArray[fd].fileBlocksAllocated += 10;	
			}

			LBAwrite(fcbArray[fd].buf, 1, fcbArray[fd].startingLBA + fileUsedBlocks);
			bytesWrite += fcbArray[fd].buflen;
			fcbArray[fd].buflen = 0;
			fileUsedBlocks += 1; // We have used 1 more block
			fcbArray[fd].cursorInDisk += 1;	// Move the cursor in disk to the next block
			fcbArray[fd].fileSize += MINBLOCKSIZE; // Increment the file size by 512

			// Update the directory LBA after getting a new LBA for where the file is located in case of a crash before the b_close
			setFileSize(fcbArray[fd].fileName, fcbArray[fd].fileSize);
			setFileBlocks(fcbArray[fd].fileName, fcbArray[fd].fileBlocksAllocated);
			setFileLBA(fcbArray[fd].fileName, fcbArray[fd].startingLBA);
		}
	}

	// Update indicator to indicate that the write buffer is holding some bytes that haven't been written
	if (fcbArray[fd].buflen != 0) {
		fcbArray[fd].writeBufferNonEmpty = true;
	}

	return pointer;
	}



// Interface to read a buffer

// Filling the callers request is broken into three parts
// Part 1 is what can be filled from the current buffer, which may or may not be enough
// Part 2 is after using what was left in our buffer there is still 1 or more block
//        size chunks needed to fill the callers request.  This represents the number of
//        bytes in multiples of the BUFSIZE.
// Part 3 is a value less than BUFSIZE which is what remains to copy to the callers buffer
//        after fulfilling part 1 and part 2.  This would always be filled from a refill 
//        of our buffer.
//  +-------------+------------------------------------------------+--------+
//  |             |                                                |        |
//  | filled from |  filled direct in multiples of the block size  | filled |
//  | existing    |                                                | from   |
//  | buffer      |                                                |refilled|
//  |             |                                                | buffer |
//  |             |                                                |        |
//  | Part1       |  Part 2                                        | Part3  |
//  +-------------+------------------------------------------------+--------+
int b_read (int fd, char * buffer, int count)
	{
	int bytesRead;				// for our reads
	int bytesReturned;			// what we will return
	int part1, part2, part3;	// holds the three potential copy lengths
	int numberOfBlocksToCopy;	// holds the number of whole blocks that are needed
	int remainingBytesInMyBuffer;	// holds how many bytes are left in my buffer	
	int remainingBytesToRead;	// = file size - bytesRead

	if (startup == 0) b_init();  //Initialize our system

	// check if fd is valid
	if ((fd < 0) || (fd >= MAXFCBS))
		{
		return (-1); 					//invalid file descriptor
		}
		
	if (fcbArray[fd].fcbStatus == -1)		//File not open for this descriptor
		{
		return -1;
		}	
		
	// If the file is not for read, stop and return error
	if (fcbArray[fd].readWriteFlags != O_RDONLY && fcbArray[fd].readWriteFlags != O_RDWR) {
		printf("ERROR: No access to read.\n");
		return -1;
	}

	// To handle EOF - reset count as the remaining bytes
	remainingBytesToRead = fcbArray[fd].fileSize - fcbArray[fd].bytesRead;
	if (count > remainingBytesToRead) {
		count = remainingBytesToRead;
	}
	
	// number of bytes available to copy from buffer
	remainingBytesInMyBuffer = fcbArray[fd].buflen - fcbArray[fd].index;	
	
	// Part 1 is The first copy of data which will be from the current buffer
	// It will be the lesser of the requested amount or the number of bytes that remain in the buffer
	
	if (remainingBytesInMyBuffer >= count)  	//we have enough in buffer
		{
		part1 = count;		// completely buffered (the requested amount is smaller than what remains)
		part2 = 0;
		part3 = 0;			// Do not need anything from the "next" buffer
		}
	else
		{
		part1 = remainingBytesInMyBuffer;				//spanning buffer (or first read)
		
		// Part 1 is not enough - set part 3 to how much more is needed
		part3 = count - remainingBytesInMyBuffer;		//How much more we still need to copy
		
		// The following calculates how many 512 bytes chunks need to be copied to
		// the callers buffer from the count of what is left to copy
		numberOfBlocksToCopy = part3 / BUFSIZE;  //This is integer math
		part2 = numberOfBlocksToCopy * BUFSIZE; 
		
		// Reduce part 3 by the number of bytes that can be copied in chunks
		// Part 3 at this point must be less than the block size
		part3 = part3 - part2; // This would be equivalent to part3 % BUFSIZE		
		}

	if (part1 > 0)	// memcpy part 1
		{
		memcpy (buffer, fcbArray[fd].buf + fcbArray[fd].index, part1);
		fcbArray[fd].index = fcbArray[fd].index + part1;
		}
		
	if (part2 > 0) 	//blocks to copy direct to callers buffer
		{
		// LBAread always returns 0, we will assume it succeeds
		LBAread (buffer+part1, numberOfBlocksToCopy, fcbArray[fd].startingLBA + fcbArray[fd].cursorInDisk); 
		bytesRead = MINBLOCKSIZE * numberOfBlocksToCopy;
		part2 = bytesRead;  //might be less if we hit the end of the file
		fcbArray[fd].cursorInDisk += numberOfBlocksToCopy; 
		}
		
	if (part3 > 0)	//We need to refill our buffer to copy more bytes to user
		{	
		//Read 1 block into our buffer
		// LBAread always returns 0, we will assume it succeeds
		LBAread (fcbArray[fd].buf, 1, fcbArray[fd].startingLBA + fcbArray[fd].cursorInDisk);
		bytesRead = MINBLOCKSIZE * 1;
		fcbArray[fd].cursorInDisk += 1;	//Move to the next block
		
		// we just did a read into our buffer - reset the offset and buffer length.
		fcbArray[fd].index = 0;
		fcbArray[fd].buflen = bytesRead; //how many bytes are actually in buffer
		
		if (bytesRead < part3) // not even enough left to satisfy read request from caller
			part3 = bytesRead;
			
		if (part3 > 0)	// memcpy bytesRead
			{
			memcpy (buffer+part1+part2, fcbArray[fd].buf + fcbArray[fd].index, part3);
			fcbArray[fd].index = fcbArray[fd].index + part3; //adjust index for copied bytes
			}	
		}
	bytesReturned = part1 + part2 + part3;
	fcbArray[fd].bytesRead += bytesReturned;
	return (bytesReturned);	
	}

// Interface to seek a specific position in file
// We will move the cursorInDisk accordingly
int b_seek(int fd, off_t offset, int whence){
	switch (whence)
	{
	/* Set the offset to 0 where the file begin 
	   And to move the positions from the beginning of the file. */
	case SEEK_SET:
		offset = 0;
		fd += offset;
		break;
	// To add the current position based on offset and write to disk.
	case SEEK_CUR:
		offset += fd;		
		LBAwrite(fcbArray[fd].buf, fd, fcbArray[fd].cursorInDisk);	
		break;
	// To move the positions from the end of the file and write to disk.
	case SEEK_END:
		offset += fd;
		LBAwrite(fcbArray[fd].buf, fd, fcbArray[fd].cursorInDisk);
		break;
	}
	LBAread(fcbArray[fd].buf, fd, fcbArray[fd].cursorInDisk);
	return offset;
}

// Interface to Close the file	
void b_close (int fd)
{
	// Add the end of the file remained in the write buffer
	// When the indicator is true, we know that there are some remaining to write
	if (fcbArray[fd].writeBufferNonEmpty) {

		// Convert fileSize to how many blocks the file is taking
		int fileUsedBlocks = ceil(fcbArray[fd].fileSize / 512);
		// check if all allocated blocks have been used
		if (fileUsedBlocks == fcbArray[fd].fileBlocksAllocated) { // if all have been used
			u_int64_t res = expandFreeSection(fcbArray[fd].startingLBA, fcbArray[fd].fileBlocksAllocated, fcbArray[fd].fileBlocksAllocated + GETMOREBLOCKS);
			if (res == 0) {
				printf("ERROR: Write failed");
			}
			// Update LBA and blocks allocated in fcb
			fcbArray[fd].startingLBA = res;
			fcbArray[fd].fileBlocksAllocated += 10;	
		}

		LBAwrite(fcbArray[fd].buf, 1, fcbArray[fd].startingLBA + fileUsedBlocks);

		// update
		fcbArray[fd].fileSize += fcbArray[fd].buflen;
		fcbArray[fd].writeBufferNonEmpty = false;
	}
	// update a few meta info about the file to directory
	setFileSize(fcbArray[fd].fileName, fcbArray[fd].fileSize);
	setFileBlocks(fcbArray[fd].fileName, fcbArray[fd].fileBlocksAllocated);
	setFileLBA(fcbArray[fd].fileName, fcbArray[fd].startingLBA);

	// reset fcb values
	fcbArray[fd].buflen = 0;
	fcbArray[fd].index = 0;
	fcbArray[fd].cursorInDisk = -1;
	fcbArray[fd].fileBlocksAllocated = 0;
	fcbArray[fd].fileSize = 0;
	fcbArray[fd].bytesRead = 0;

	free (fcbArray[fd].buf);			// free the associated buffer
	fcbArray[fd].buf = NULL;			// Safety First
	fcbArray[fd].fcbStatus = -1;			// return this FCB to list of available FCB's 
}