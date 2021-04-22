/**************************************************************
* Class:  CSC-415
* Name: Jinjian Tan
* Student ID: N/A
* Project: Basic File System Test Driver
*
* File: testDriver.c
*
* Description: This is a driver file to test the file system.
* 
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
#include "vcb.h"
int main (int argc, char *argv[])
	{	
	char * filename = "VolumeZero";
	uint64_t volumeSize = 10000000;
	uint64_t blockSize = BLOCKSIZE;
  int retVal;
		
	retVal = startPartitionSystem (filename, &volumeSize, &blockSize);	
	printf("Opened %s, Volume Size: %llu;  BlockSize: %llu; Return %d\n", filename, (ull_t)volumeSize, (ull_t)blockSize, retVal);
	
	char * buf = malloc(blockSize *2);
	char * buf2 = malloc(blockSize *2);

	// memset (buf, 0, blockSize*2);

	vcb *v0 = initializeVCB(BLOCKSIZE*10, BLOCKSIZE);
	vcb *v1 = malloc(sizeof(vcb));
	memcpy(buf, v0, sizeof(vcb));
	// printf("size of v0 is %lu\n", sizeof(v0));
	// printf("size of v1 is %lu\n", sizeof(v1));
	LBAwrite (buf, 2, 0);
	LBAread (buf2, 2, 0);
	memcpy(v1, buf2, sizeof(vcb));
	// memcpy(v1, buf2, sizeof(vcb));
	printf("magic number is %x\n", v1->magic_number);
	if (memcmp(v0, v1, sizeof(vcb))==0)
		{
		printf("Read/Write worked\n");
		}
	else
		printf("FAILURE on Write/Read\n");
		
	free (buf);
	free(buf2);
	closePartitionSystem();
	return 0;	
	}
	
