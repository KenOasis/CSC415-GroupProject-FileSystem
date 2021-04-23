#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include "b_io.h"

#define MAXFCBS 20
#define BUFSIZE 512

typedef struct b_fcb
{
	int systemFd;	//holds the systems file descriptor
	char * buf;		//holds the open file buffer
	int index;		//holds the current position in the buffer
	int buflen;		//holds how many valid bytes are in the buffer
	bool writeBufferNonEmpty; //track if we need to flush write buffer
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
	
	// lets try to open the file before I do too much other work
	
	fd = open (filename, flags, S_IRWXU | S_IRGRP | S_IROTH);
	if (fd  == -1)
		return (-1);		//error opening filename
		
	//Should have a mutex here
	returnFd = b_getFCB();				// get our own file descriptor
										// check for error - all used FCB's
	fcbArray[returnFd].systemFd = fd;	// Save the linux file descriptor
	//	release mutex
	
	//allocate our buffer
	fcbArray[returnFd].buf = malloc (BUFSIZE);
	if (fcbArray[returnFd].buf  == NULL)
	{
		// very bad, we can not allocate our buffer
		close (fd);							// close linux file
		fcbArray[returnFd].systemFd = -1; 	//Free FCB
		return -1;
	}
		
	fcbArray[returnFd].buflen = 0; 			// have not read anything yet
	fcbArray[returnFd].index = 0;			// have not read anything yet

	return (returnFd);						// all set
}

// Interface to Close the file	
void b_close (int fd)
{
	close (fcbArray[fd].systemFd);		// close the linux file handle
	free (fcbArray[fd].buf);			// free the associated buffer
	fcbArray[fd].buf = NULL;			// Safety First
	fcbArray[fd].systemFd = -1;			// return this FCB to list of available FCB's 
}
