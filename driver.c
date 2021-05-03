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
#include "b_io.h"

int main (int argc, char *argv[])
	{	
	char * filename = "SampleVolume";
	uint64_t volumeSize = 10000000;
	uint64_t blockSize = BLOCKSIZE;
    int retVal;
	int testfs_fd;
	int bytesWritten;
	int bytesRead;
		
	retVal = startPartitionSystem (filename, &volumeSize, &blockSize);	
	printf("Opened %s, Volume Size: %llu;  BlockSize: %llu; Return %d\n", filename, (ull_t)volumeSize, (ull_t)blockSize, retVal);
	char * buf = malloc(blockSize *2);
	vcb v0 = initializeVCB(BLOCKSIZE * 2, BLOCKSIZE);
	memcpy(buf, &v0, sizeof(v0));
	LBAwrite (buf, 2, 0);

	// // TEST b_write()
	// // open a file
	// testfs_fd = b_open ("testfile", O_WRONLY | O_CREAT | O_TRUNC);
	// printf("testfs_fd: %d\n", testfs_fd);

	// // write a paragraph to the file
	// char * buf_write = "Tuxedo cats are best known for their bi-colored coats that look like, well, tiny tuxedos. Although many tuxedo cats are black and white, these gorgeous kitties' coats can also be gray, silver, orange, and even tortoiseshell with patches of white. But there's a lot more to tuxedo cats than their good looks. The richest cat in the world, for example, was a tuxedo cat. And other tuxedo cats have been to war, the top of Mount Everest, and the White House. Yes, really! Want more? Check out these fascinating facts about nature's most dapper kitties.";
	// bytesWritten = b_write(testfs_fd, buf_write, strlen(buf_write));
	// printf("bytesWritten: %d\n", bytesWritten);
	// b_close(testfs_fd);	// when close, write the rest bytes in buffer to the file

	// TEST b_read()
	char * buf_write2 = "Tuxedo cats are best known for their bi-colored coats that look like, well, tiny tuxedos. ";
	char * buf_read = malloc(blockSize *1);

	testfs_fd = b_open ("testfile", O_RDONLY | O_CREAT | O_TRUNC);
	LBAwrite(buf_write2, 1, 1);
	bytesRead = b_read(testfs_fd, buf_read, 512);
	printf("bytesRead: %d\n", bytesRead);
	printf("buf_read: %s\n", buf_read);

	free(buf);
	closePartitionSystem();
	return 0;	
	}
	
