#ifndef _FREESPACE_H
#define _FREESPACE_H

#include <stdlib.h>
#include <stdio.h>
#include "fsLow.h"

typedef struct  {
	int* bitVector;
	int size;
	uint64_t LBABitVector; //LBA location of bit vector
	int blocksNeeded;
	int structSize;
} freeSpace;

freeSpace* init_freeSpace(int totalBlocks, int bytesPerBlock); //initialize the free space if it doesn't exist
u_int64_t findMultipleBlocks(int blockCount); //find free blocks and return the LBA
void freeSomeBits(int startIndex, int count); //free a chunk of blocks
u_int64_t expandFreeSection(int fileLocation, int fileBlockSize, int newBlockSize); //expand the file to new size, move file if current location can't be allocated
#endif
