#ifndef _FREESPACE_H
#define _FREESPACE_H

#include <stdlib.h>
#include <stdio.h>
#include "fsLow.h"

typedef struct  {
	int* bitVector;
	//int nextFreePosition;
	//int nextFreeIndex;
	int size;
	uint64_t LBABitVector; //LBA location of bit vector
	int blocksNeeded;
	int structSize;
} freeSpace;

freeSpace* init_freeSpace(int totalBlocks, int bytesPerBlock);
//u_int64_t findFreeBlock(freeSpace* vector);
u_int64_t findMultipleBlocks(int blockCount);
void freeSomeBits(int startIndex, int count);
u_int64_t expandFreeSection(int fileLocation, int fileBlockSize, int newBlockSize);
#endif
