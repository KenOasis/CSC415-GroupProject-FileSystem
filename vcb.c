#include "vcb.h"
#include "mfs.h"
#include "freeSpace.h"
vcb initializeVCB(uint64_t volumesize, uint64_t blocksize) {
    int number_of_blocks = (volumesize + BLOCKSIZE - 1) / BLOCKSIZE;
    vcb *v0 = malloc(BLOCKSIZE);
    // Read from disk

    // initialize 
   v0 -> number_of_blocks = number_of_blocks;
   v0 -> size_of_block = BLOCKSIZE;
   v0 -> total_size_in_bytes = v0 -> number_of_blocks * v0 -> size_of_block;
   v0 -> magic_number = 0x12345678;
   v0 -> number_of_free_blocks = v0 -> number_of_blocks;
   // initialize free space
   v0 -> free_space_ptr = init_freeSpace(number_of_blocks, BLOCKSIZE);
   v0 -> LBA_free_space = 1;
   // initialize directory
   v0 -> LBA_root_directory = 1;

   // Write to disk
   LBAwrite(v0,1,0);
    return *v0;
};
