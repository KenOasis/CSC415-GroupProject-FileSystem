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

freeSpace* vector;

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
	vector = init_freeSpace(1000, 512);
	u_int64_t blockLocation = findMultipleBlocks(5);
	char * buffer = malloc(1024);
	buffer = "This is some random information";
	u_int64_t blockLocationTwo = findMultipleBlocks(2);
	LBAwrite(buffer, 2, blockLocationTwo);
	blockLocation = findMultipleBlocks(6);
	//freeSomeBits(30, 2, space);
	blockLocationTwo = expandFreeSection(blockLocationTwo, 2, 9);
	for (int n = 0; n < vector->size; n++) {
		printf("Integer %d: %d\n", n, vector->bitVector[n]);
	}
	printf("Free block location: %lu, Second: %lu\n", blockLocation, blockLocationTwo);
	LBAwrite(vector, 1, 2);
	LBAwrite(vector->bitVector, (vector->size + 127) / 128, vector->LBABitVector);
	printf("size of freespace: %ld\n", sizeof(freeSpace));
	free(vector->bitVector);
	free(vector);
	vector = NULL;
	vector = malloc(512); //resets the freespace struct and vector, as if partition shut down. read from disk and see if it's consistent
	LBAread(vector, 1, 2);
	printf("BlocksNeeded: %d\n", vector->blocksNeeded);
	printf("Array int size: %d\n", vector->size);
	vector->bitVector = malloc(sizeof(int) * vector->size);
	LBAread(vector->bitVector, (vector->size + 127) / 128, vector->LBABitVector);
	findMultipleBlocks(3); //make a change to see if new vector from disk is usable
	for (int n = 0; n < vector->size; n++) {
		printf("Integer %d: %d\n", n, vector->bitVector[n]);
	}
	free (buf);
	free(buf2);
	free(vector->bitVector);
	free(vector);
	closePartitionSystem();
	return 0;	
	}
	
