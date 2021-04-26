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

	vcb *v0 = bootVCB(blockSize*10, blockSize);
	printf("magic number is:%x\n", v0->magic_number);
	fs_directory* directory = malloc(blockSize);
	LBAread(directory, 1, v0->LBA_root_directory);
	reload_directory(directory);
	char *name = "root/Users/Jimmy";
	fdDir *fd = fs_opendir(name);
	free(fd);
	free_directory(directory);
	free(v0);
	closePartitionSystem();
	return 0;	
	}
	
