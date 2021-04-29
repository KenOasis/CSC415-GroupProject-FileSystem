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
//#include "vcb.h"
#include "freeSpace.h"

int main (int argc, char *argv[])
	{	
	char * filename = "SampleVolume";
	uint64_t volumeSize = 50000;
	uint64_t blockSize = 512;
  int retVal;
		
	retVal = startPartitionSystem (filename, &volumeSize, &blockSize);	
	printf("Opened %s, Volume Size: %llu;  BlockSize: %llu; Return %d\n", filename, (ull_t)volumeSize, (ull_t)blockSize, retVal);
	
	char * buf = malloc(blockSize *2);
	char * buf2 = malloc(blockSize *2);

	// memset (buf, 0, blockSize*2);
/*
	vcb v0 = initializeVCB(512 * 2, 512);

	memcpy(buf, &v0, sizeof(v0));
	LBAwrite (buf, 2, 0);
	LBAread (buf2, 2, 0);
	vcb *v1 = malloc(sizeof(vcb));
	memcpy(v1, buf2, sizeof(vcb));
	printf("magic number is %x\n", v1->magic_number);
	if (memcmp(buf, buf2, blockSize*2)==0)
		{
		printf("Read/Write worked\n");
		}
	else
		printf("FAILURE on Write/Read\n");
		
*/
	freeSpace* space = init_freeSpace(1000, 512);
	u_int64_t blockLocation = findMultipleBlocks(5, space);
	u_int64_t blockLocationTwo = findMultipleBlocks(2, space);
	blockLocation = findMultipleBlocks(6, space);
	//freeSomeBits(30, 2, space);
	blockLocationTwo = expandFreeSection(blockLocationTwo, 2, 9, space);
	for (int n = 0; n < space->size; n++) {
		printf("Integer %d: %d\n", n, space->bitVector[n]);
	}
	printf("Free block location: %lu, Second: %lu\n", blockLocation, blockLocationTwo);
	LBAwrite(space, 1, 2);
	LBAwrite(space->bitVector, space->size / 128, space->LBABitVector);
	freeSpace* spaceCopy = malloc(sizeof(freeSpace));
	LBAread(spaceCopy, 1, 2);
	printf("BlocksNeeded: %d\n", spaceCopy->blocksNeeded);
	printf("Array int size: %d\n", spaceCopy->size);
	spaceCopy->bitVector = malloc(sizeof(int) * 300);
	/*
	LBAread(spaceCopy->bitVector, spaceCopy->size / 128, space->LBABitVector);
	for (int n = 0; n < spaceCopy->size; n++) {
		printf("Integer %d: %d\n", n, spaceCopy->bitVector[n]);
	}*/
	free (buf);
	free(buf2);
	closePartitionSystem();
	return 0;	
	}
	
