#ifndef _FREESPACE_H
#define _FREESPACE_H

#include <stdlib.h>

typedef struct  {
	int* bitVector;
	int nextFreePosition;
	int nextFreeIndex;
	int size;
} freeSpace;

freeSpace* init_freeSpace(int totalBlocks, int bytesPerBlock);
int findFreeBlock(int space, freeSpace* vector);

#endif
