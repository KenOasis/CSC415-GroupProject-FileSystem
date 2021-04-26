#include "freeSpace.h"

freeSpace* init_freeSpace(int totalBlocks, int BytesPerBlock) {
	freeSpace *vector = malloc(sizeof(freeSpace));
	vector->LBABitVector = ((sizeof(freeSpace) + 511) / 512) + 2; //size of freeSpace struct without the array
	int integers = (totalBlocks + 31) / 32; //how many integers in the int array
	//printf("integers needed: %d\n", integers);
	int blocksNeeded = ((integers * 32) + BytesPerBlock * 8 - 1) / (BytesPerBlock * 8); //amount of BITS needed for free space bit vector
	blocksNeeded += vector->LBABitVector; //add VCB needed block, and freeSpace struct needed blocks
	//printf("blocks needed: %d\n", blocksNeeded);
	vector->size = integers; //length of the bit vector for printing or iterating
	vector->blocksNeeded = (integers + 15) / 16;
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
	//vector->nextFreePosition = 0;
	//vector->nextFreeIndex = blocksNeeded; //next free block is right after free space bit vector
	return vector;
}

u_int64_t findMultipleBlocks(int blockCount, freeSpace* vector) {
	int consecutiveFree = 0; 						//tracks how many free spaces have been found in a row
	int freeIndex = 0; 							//the bit index that is free, followed by more free space
	for (int n = 0; n < vector->size; n++) { 		//iterates through the ints in the bitvector
		for (int a = 0; a < 32; a++) { 				//iterates through the bits in bitvector
			if ((vector->bitVector[n] & (1 << a)) == 0) {		//if vector bit is 0 aka free
				if (consecutiveFree == 0) {					//checks if this is the first free bit found, or following free bits
					freeIndex = n * 32 + a;
					consecutiveFree += 1;
					//printf("FREE BLOCKS werE FOUND\n");
				} else {
					consecutiveFree += 1;
				}
			} else {								//chain of free bits not big enough
				consecutiveFree = 0;
				freeIndex = 0;
			}
			if (consecutiveFree >= blockCount) {	//found the necessary chunk, sets all used bits to 1 and return the index of first free bit
				//printf("FREE BLOCKS werE FOUND\n");
				int count = consecutiveFree;
				int position = freeIndex;
				int freePos = position / 32;
				int freeBit = position % 32;
				while (count > 0) {
					vector->bitVector[freePos] |= 1 << freeBit;
					count -= 1;
					freeBit += 1;
					if (freeBit >= 32) {
						freePos += 1;
						freeBit = 0;
					}
				}
				LBAwrite(vector->bitVector, vector->size, 1 + vector->LBABitVector);
				//printf("free block found: %d\n", freeIndex);
				return freeIndex;
			}
		}
	}
	//printf("free block NOT found: %d\n", freeIndex);
	return freeIndex; 								//bits were not found, return -1 to indicate error
}

void freeSomeBits(int startIndex, int count, freeSpace* vector) {
	int index = startIndex / 32;
	int position = startIndex % 32;
	for (int n = 0; n < count; n++) {
		int value = 1 << position;
		value = ~value;
		vector->bitVector[index] &= value;
		position += 1;
		if (position >= 32) {
			index += 1;
			position = 0;
		}
	}
}

u_int64_t expandFreeSection(int index, freeSpace* vector) {
	int freeIndex = index;
	int expandAmount = 0;
	return freeIndex;
}

int main() {
	freeSpace* space = init_freeSpace(500, 512);
	u_int64_t blockLocation = findMultipleBlocks(5, space);
	u_int64_t blockLocationTwo = findMultipleBlocks(2, space);
	freeSomeBits(4, 5, space);
	blockLocation = findMultipleBlocks(5, space);
	freeSomeBits(30, 2, space);
	for (int n = 0; n < space->size; n++) {
		printf("Integer %d: %d\n", n, space->bitVector[n]);
	}
	printf("Free block location: %lu, Second: %lu\n", blockLocation, blockLocationTwo);
	//printf("SIZE WITHOUT VECTOR: %lu\n", space->LBABitVector);
	return 0;
}
/*u_int64_t findFreeBlock(freeSpace* vector) {
	int freeIndex = vector->nextFreeIndex + vector->nextFreePosition * 32; //the bit number that is free, aka the next free block
	vector->bitVector[vector->nextFreePosition] |= 1 << vector->nextFreeIndex; //sets bit to 1 for future reference
	vector->nextFreeIndex += 1;
	if (vector->nextFreeIndex >= 32) { //if nextFreeIndex >= 32, then move to next int in the bit vector
		vector->nextFreePosition += 1;
		vector->nextFreeIndex = 0;
	}
	if (vector->nextFreePosition == vector->size) { //freeSpace is full
		return -1;
	}
	return freeIndex;
}

u_int64_t findMultipleBlocks(int blockCount, freeSpace* vector) {
	int count = blockCount; //tracks how many remaining blocks to reserve/handle
	int freeIndex = vector->nextFreeIndex + vector->nextFreePosition * 32; //the bit index that is free, followed by more free space
	while (count > 0) { //counts down and manipulates the bits that are free that will be used, returns -1 if not enough bits/blocks left
		vector->bitVector[vector->nextFreePosition] |= 1 << vector->nextFreeIndex; //set bit to 1 for future reference
		vector->nextFreeIndex += 1;
		if (vector->nextFreeIndex >= 32) { //if nextFreeIndex >= 32, then move to next int in the bit vector
                	vector->nextFreePosition += 1;
                	vector->nextFreeIndex = 0;
        	}
		if (vector->nextFreePosition == vector->size) { //free space is full
			return -1;
		}
		count--;
	}
	return freeIndex;
}*/