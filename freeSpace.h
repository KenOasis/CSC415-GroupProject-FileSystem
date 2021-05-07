/**************************************************************
Class:  CSC-415-02 Spring 2021
* Names: Zhuozhuo Liu, Jinjian Tan, Yunhao Wang (Mike on iLearn), Chu Cheng Situ
* Student ID: 921410045 (Zhuozhuo), 921383408 (Jinjian), 921458509 (Yunhao), 921409278 
* (Chu Cheng)
* GitHub Names: liuzz10 (Zhuozhuo), KenOasis (Jinjian), mikeyhwang (Yunhao), chuchengsitu 
* (Chu Cheng)
* Group Name: return 0
* Project: Term Project – File System
*
* File: freeSpace.h
*
* Description: the header file containing all necessary functions
* of the free space, including the ability to find free blocks
* free some blocks, and expand an existing file
*              
*
**************************************************************/
#ifndef _FREESPACE_H
#define _FREESPACE_H

#include <stdlib.h>
#include <stdio.h>
#include "fsLow.h"

typedef struct  {
	int* bitVector; //int bit vector that is the free space
	int size; //the amount of ints in the bit vector
	uint64_t LBABitVector; //LBA location of bit vector
	int blocksNeeded; //blocks needed by the bit vector
	int structSize;
	int blockCount; //amount of blocks vector is accounting for
} freeSpace;

freeSpace* init_freeSpace(int totalBlocks, int bytesPerBlock); //initialize the free space if it doesn't exist
u_int64_t findMultipleBlocks(int blockCount); //find free blocks and return the LBA
void freeSomeBits(int startIndex, int count); //free a chunk of blocks
u_int64_t expandFreeSection(int fileLocation, int fileBlockSize, int newBlockSize); //expand the file to new size, move file if current location can't be allocated
#endif
