#ifndef _FREESPACE_H
#define _FREESPACE_H

#include <stdlib.h>

typedef struct  {
	int* bitVector;
	int nextFreePosition;
	int nextFreeIndex;
	int size;
	int blocksNeeded;
} freeSpace;

freeSpace* init_freeSpace(int totalBlocks, int bytesPerBlock);
u_int64_t findFreeBlock(freeSpace* vector);
u_int64_t findMultipleBlocks(int blockCount, freeSpace* vector);

#endif
