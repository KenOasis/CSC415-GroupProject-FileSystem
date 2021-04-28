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
#include "fsLow.h"  // import MINBLOCKSIZE 512

#define MAXFCBS 20

typedef struct b_fcb
	{
	int systemFd;	//holds the systems file descriptor - the LBA of the first block
	char * buf;		//holds the open file buffer
	int index;		//holds the current position in the buffer
	int buflen;		//holds how many valid bytes are in the buffer
	bool writeBufferNonEmpty;	// track if we need to flush write buffer
	int blockInDisk	// the cursor that tracks where we are
	} b_fcb;
	
b_fcb fcbArray[MAXFCBS];

int startup = 0;	//Indicates that this has not been initialized




// Interface to write a buffer	
int b_write (int fd, char * buffer, int count)
	{
	int bytesWrite;				// how many bytes written to the file, there can be some left in the buffer
	int bytesNeededToFull;		// how many bytes are left in my buffer to fill up the buffer
	int pointer;				// how many bytes have been processed in the call's buffer
	int loadToBuffer;			// how many bytes is going to be load to buffer

	if (startup == 0) b_init();  //Initialize our system

	// check that fd is between 0 and (MAXFCBS-1)
	if ((fd < 0) || (fd >= MAXFCBS))
		{
		return (-1); 					//invalid file descriptor
		}
		
	if (fcbArray[fd].systemFd == -1)		//File not open for this descriptor
		{
		return -1;
		}	

	// Initialize the pointer pointing to the first byte in caller's buffer
	pointer = 0;
	
	// Writing to the file from caller's buffer until reaching the end
	while (count > 0) {
		// number of bytes needed to fill the buffer
		bytesNeededToFull = MINBLOCKSIZE - fcbArray[fd].buflen;
		
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
		if (fcbArray[fd].buflen == MINBLOCKSIZE) {
			freespace = //
			fcbArray[fd].blockInDisk = findMultipleBlocks(1, freespace);
			LBAwrite(fcbArray[fd].buf, fcbArray[fd].buflen, fcbArray[fd].systemFd + fcbArray[fd].blockInDisk);
			bytesWrite += fcbArray[fd].buflen;
			fcbArray[fd].buflen = 0;
			fcbArray[fd].blockInDisk += 1;	// Move the block in disk to the next
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
//        bytes in multiples of the MINBLOCKSIZE.
// Part 3 is a value less than MINBLOCKSIZE which is what remains to copy to the callers buffer
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
	
	if (startup == 0) b_init();  //Initialize our system

	// check if fd is valid
	if ((fd < 0) || (fd >= MAXFCBS))
		{
		return (-1); 					//invalid file descriptor
		}
		
	if (fcbArray[fd].systemFd == -1)		//File not open for this descriptor
		{
		return -1;
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
		numberOfBlocksToCopy = part3 / MINBLOCKSIZE;  //This is integer math
		part2 = numberOfBlocksToCopy * MINBLOCKSIZE; 
		
		// Reduce part 3 by the number of bytes that can be copied in chunks
		// Part 3 at this point must be less than the block size
		part3 = part3 - part2; // This would be equivalent to part3 % MINBLOCKSIZE		
		}
				
	if (part1 > 0)	// memcpy part 1
		{
		memcpy (buffer, fcbArray[fd].buf + fcbArray[fd].index, part1);
		fcbArray[fd].index = fcbArray[fd].index + part1;
		}
		
	if (part2 > 0) 	//blocks to copy direct to callers buffer
		{
		bytesRead = LBAread (buffer+part1, numberOfBlocksToCopy, fcbArray[fd].systemFd + fcbArray[fd].blockInDisk);
		part2 = bytesRead;  //might be less if we hit the end of the file
		fcbArray[fd].blockInDisk += numberOfBlocksToCopy; //
		}
		
	if (part3 > 0)	//We need to refill our buffer to copy more bytes to user
		{		
		//try to read MINBLOCKSIZE bytes into our buffer
		bytesRead = LBAread (fcbArray[fd].buf, MINBLOCKSIZE, fcbArray[fd].systemFd + fcbArray[fd].blockInDisk);
		fcbArray[fd].blockInDisk += 1;	//Move to the next block
		
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
	return (bytesReturned);	
	}
