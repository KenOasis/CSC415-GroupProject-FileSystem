#include "freeSpace.h"
typedef struct vcb {
    int number_of_blocks;  // number of blocks in volume
    int size_of_block;  // the size of a single block
    int total_size_in_bytes;  // the product of number_of_blocks and size_of_block
    uint64_t LBA_free_space;  // the LBA of free space
    uint64_t LBA_root_directory;  // the LBA of the root directory
    int magic_number;  // magic number: where to write what value?
    int number_of_free_blocks;  // number of free blocks
    freeSpace* free_space_ptr;  // not persistent
} vcb;

#define BLOCKSIZE 512

/*
 * @brief initializes VCB
 * @param volumesize The size of the the whole volume
 * @param blocksize The size of the the whole volume
 * @return void
 */
int initializeVCB(uint64_t volumesize, uint64_t blocksize);
vcb* bootVCB(uint64_t volumesize, uint64_t blocksize);