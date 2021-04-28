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

	vcb *v0 = bootVCB(blockSize*10240, blockSize);
	printf("magic number is:%x\n", v0->magic_number);
	fs_directory* directory = malloc(blockSize);
	LBAread(directory, 1, v0->LBA_root_directory);
	reload_directory(directory);
	char *name = "root/Users/";
	fdDir *fdir = fs_opendir(name);
	if(fdir != NULL){
		printf("children number is %u\n", fdir->num_children);
		int file_count =0;
		struct fs_diriteminfo *dirinfo;
		while((dirinfo = fs_readdir(fdir)) != NULL){
			file_count++;
			printf("file %d: %s\n", file_count, dirinfo->d_name);
		}
		free(dirinfo);
		char *newpath = malloc(sizeof(char) * 256);
		char *origpath = malloc(sizeof(char) * 128);
		strcpy(origpath, name);
		newpath = strcat(origpath,"Another file");
		struct fs_stat *buf = malloc(sizeof(struct fs_stat));
		fs_stat("Jimmy", buf);
		printf(" Jimmy size is %lld\n", buf->st_size);
	}	
	if(fdir != NULL){
		free(fdir);
	}
	free_directory(directory);
	closePartitionSystem();
	return 0;	
	}
	
