#include "freeSpace.h"
freeSpace* init_freeSpace(int totalBlocks, int BytesPerBlock) {
	freeSpace *vector = malloc(sizeof(freeSpace));
	int integers = (totalBlocks + 31) / 32; //how many integers in the int array
	int blocksNeeded = ((integers * 32) + BytesPerBlock * 8 - 1) / (BytesPerBlock * 8); //amount of BITS needed for free space bit vector
	blocksNeeded += 1; //add VCB needed block
	vector->size = integers; //length of the bit vector for printing or iterating
	vector->bitVector = malloc(sizeof(int) * integers);
        for (int b = 0; b < integers; b++) {
                vector->bitVector[b] = 0; //sets all of the free space array to 0
        }
	for (int n = 0; n < blocksNeeded; n++) {
		//set bits used by VCB and this bitVector to 1
		unsigned int flag = 1;
		flag = flag << n;
		vector->bitVector[0] |= flag;
	}
	vector->nextFreePosition = 0;
	vector->nextFreeIndex = blocksNeeded; //next free block is right after free space bit vector
	return vector;
}

int findFreeBlock(int space, freeSpace* vector) {
	int freeIndex = vector->nextFreeIndex + vector->nextFreePosition * 32; //the bit number that is free, aka the next free block
	vector->bitVector[vector->nextFreePosition] |= 1 << vector->nextFreeIndex; //sets bit to 1 for future reference
	vector->nextFreeIndex += 1;
	if (vector->nextFreeIndex >= 32) { //if nextFreeIndex >= 32, then move to next int in the bit vector
		vector->nextFreePosition += 1;
		vector->nextFreeIndex = 0;
	}
	return freeIndex;
}

int main(int argc, const char * argv[]) {
	return 0;
}
