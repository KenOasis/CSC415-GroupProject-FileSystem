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
		
	retVal = startPartitionSystem (filename, &volumeSize, &blockSize);	
	printf("Opened %s, Volume Size: %llu;  BlockSize: %llu; Return %d\n", filename, (ull_t)volumeSize, (ull_t)blockSize, retVal);
	
	char * buf = malloc(blockSize *2);

	vcb v0 = initializeVCB(BLOCKSIZE * 2, BLOCKSIZE);

	memcpy(buf, &v0, sizeof(v0));
	LBAwrite (buf, 2, 0);


	free(buf);

	closePartitionSystem();
	return 0;	
	}
	
