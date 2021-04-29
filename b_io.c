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
#include <math.h>	// for ceil()
#include "freeSpace.h"

#define MAXFCBS 20
#define BUFSIZE 512
#define GETMOREBLOCKS 10

typedef struct b_fcb
	{
	int systemFd;		// Each file will be assigned an id after b_open
	int startingLBA;	//the LBA of the first block
	char * buf;		//holds the open file buffer
	int index;		//holds the current position in the buffer
	int buflen;		//holds how many valid bytes are in the buffer
	bool writeBufferNonEmpty;	// track if we need to flush write buffer
	int cursorInDisk;	// the cursor that tracks which block we are on disk
	int fileSize;	// the current size of the file
	int fileBlocksAllocated;	// total number of the blocks that the file allocated
	} b_fcb;
	
b_fcb fcbArray[MAXFCBS];

int startup = 0;	//Indicates that this has not been initialized

//Method to initialize our file system
void b_init ()
{
	//init fcbArray to all free
	for (int i = 0; i < MAXFCBS; i++)
		{
		fcbArray[i].systemFd = -1; //indicates a free fcbArray
		}
		
	startup = 1;
}

//Method to get a free FCB element
int b_getFCB ()
{
	for (int i = 0; i < MAXFCBS; i++)
	{
		if (fcbArray[i].systemFd == -1)
		{
			fcbArray[i].systemFd = -2; // used but not assigned
			return i;		//Not thread safe
		}
	}
	return (-1);  //all in use
}
	
// Interface to open a buffered file
// Modification of interface for this assignment, flags match the Linux flags for open
// O_RDONLY, O_WRONLY, or O_RDWR
int b_open (char * filename, int flags)
{
	int fd;
	int returnFd;

	if (startup == 0) b_init();  //Initialize our system
	
		
	//Should have a mutex here
	returnFd = b_getFCB();				// get our own file descriptor
										// check for error - all used FCB's
	b_fcb* fcb;	// check if there was an available fcb return
	if(returnFd < 0){
		return -1;
	}

	fcb = &fcbArray[returnFd]; //open the file
	
	//allocate our buffer
	fcb -> buf = malloc (BUFSIZE);
	if (fcb -> buf  == NULL)
	{
		// very bad, we can not allocate our buffer
		close (fd);							// close linux file
		fcb->systemFd = -1; 	//Free FCB
		return -1;
	}
		
	fcb->buflen = 0; 			// have not read anything yet
	fcb->index = 0;			// have not read anything yet
    fcb->cursorInDisk = 0;	// the current block is 0
	fcb->fileBlocksAllocated = 0; //have not read anything yet


	return (returnFd);						// all set
}

int b_seek(int fd, off_t offset, int whence){
	switch (whence)
	{
	// Set the offset to 0 where the file begin
	case SEEK_SET:
		offset = 0;
		fd = offset;
		break;
	// Set the 
	case SEEK_CUR:
		fd += offset;
		fd -= offset;
		LBAread;
		LBAwrite;
		break;

	case SEEK_END:
		fd += offset;
		fd -= offset;
		LBAread;
		LBAwrite;
		break;
	}
	return offset;

}

// Interface to Close the file	
void b_close (int fd)
{
	b_fcb* fcb = &fcbArray[fd];
	// Add the end of the file remained in the write buffer
	// When the indicator is true, we know that there are some remaining to write
	if (fcbArray[fd].writeBufferNonEmpty) {
		// Convert fileSize to how many blocks the file is taking
		int fileUsedBlocks = ceil(fcbArray[fd].fileSize / 512);
		// check if all allocated blocks have been used
		if (fileUsedBlocks == fcbArray[fd].fileBlocksAllocated) { // if no empty block to use
			int res = findMultipleBlocks(fcbArray[fd].startingLBA, fcbArray[fd].fileBlocksAllocated, fcbArray[fd].fileBlocksAllocated + GETMOREBLOCKS);
			if (res == -1) {
				printf("ERROR: Write failed");
				return -1;
			}
			fcbArray[fd].startingLBA = res; // Update the startingLBA
		}
		LBAwrite(fcbArray[fd].buf, 1, fcbArray[fd].startingLBA + fileUsedBlocks);

		// update
		fcbArray[fd].fileSize += fcbArray[fd].buflen;
		fcbArray[fd].writeBufferNonEmpty = false;
	}
	// update a few info about the file
	// TODO: update file size, startingLBA to directory

	// Reset values
	fcbArray[fd].buflen = 0;
	fcbArray[fd].index = 0;
	fcbArray[fd].cursorInDisk = -1;
	fcbArray[fd].fileBlocksAllocated = 0;
	fcbArray[fd].fileSize = 0;

	close (fcb->systemFd);		// close the linux file handle
	free (fcb->buf);			// free the associated buffer
	fcb->buf = NULL;			// Safety First
	fcb->systemFd = -1;			// return this FCB to list of available FCB's 
}
