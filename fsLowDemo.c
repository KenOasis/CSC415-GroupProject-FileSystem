/**************************************************************
* Class:  CSC-415
* Name: Professor Bierman
* Student ID: N/A
* Project: Basic File System
*
* File: fsLowDemo.c
*
* Description: This is a demo to show how to use the fsLow
* 	routines.
*
**************************************************************/


#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <pthread.h>
#include <errno.h>
#include <math.h>
#include <time.h>
#include "fsLow.h"
#include "mfs.h"
#include "freeSpace.h"

int main (int argc, char *argv[])
	{	
	char * filename;
	uint64_t volumeSize;
	uint64_t blockSize;
    int retVal;
    
		filename = "SampleVolume";
		volumeSize = 50;
		blockSize = 512;
		
	retVal = startPartitionSystem (filename, &volumeSize, &blockSize);	
	printf("Opened %s, Volume Size: %llu;  BlockSize: %llu; Return %d\n", filename, (ull_t)volumeSize, (ull_t)blockSize, retVal);
	
	char * buf = malloc(blockSize *2);
	char * buf2 = malloc(blockSize *2);
	memset (buf, 0, blockSize*2);
	strcpy (buf, "Now is the time for all good people to come to the aid of their countrymen\n");
	strcpy (&buf[blockSize+10], "Four score and seven years ago our fathers brought forth onto this continent a new nation\n");
	LBAwrite (buf, 2, 0);
	LBAwrite (buf, 2, 3);
	LBAread (buf2, 2, 0);
	if (memcmp(buf, buf2, blockSize*2)==0)
		{
		printf("Read/Write worked\n");
		}
	else
		printf("FAILURE on Write/Read\n");
	
	freeSpace* space = init_freeSpace(500, 512);
	u_int64_t blockLocation = findMultipleBlocks(5, space);
	u_int64_t blockLocationTwo = findMultipleBlocks(2, space);
	freeSomeBits(4, 5, space);
	blockLocation = findMultipleBlocks(5, space);
	freeSomeBits(30, 2, space);
	for (int n = 0; n < space->size; n++) {
		printf("Integer %d: %d\n", n, space->bitVector[n]);
	}
	printf("Free block location: %lu, Second: %lu\n", blockLocation, blockLocationTwo);
	free (buf);
	free(buf2);
	closePartitionSystem();
	return 0;	
	}
	
