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
#include "vcb.h"
int main (int argc, char *argv[])
	{	
	char * filename = "SampleVolume";
	uint64_t volumeSize = 10000000;
	uint64_t blockSize = BLOCKSIZE;
  int retVal;
		
	retVal = startPartitionSystem (filename, &volumeSize, &blockSize);	
	printf("Opened %s, Volume Size: %llu;  BlockSize: %llu; Return %d\n", filename, (ull_t)volumeSize, (ull_t)blockSize, retVal);
	
	char * buf = malloc(blockSize *2);
	char * buf2 = malloc(blockSize *2);

	// memset (buf, 0, blockSize*2);

	vcb v0 = initializeVCB(BLOCKSIZE * 2, BLOCKSIZE);

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
		
	free (buf);
	free(buf2);
	closePartitionSystem();
	return 0;	
	}
	
