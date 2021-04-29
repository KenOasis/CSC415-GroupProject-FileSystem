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
u_int64_t findMultipleBlocks(int blockCount, freeSpace* vector);
void freeSomeBits(int startIndex, int count, freeSpace* vector);
u_int64_t expandFreeSection(int fileLocation, int fileSize, int newSize, freeSpace* vector);
#endif
