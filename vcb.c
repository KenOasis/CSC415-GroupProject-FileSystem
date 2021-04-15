#include "vcb.h"
#include "mfs.h"
#include "freeSpace.h"
vcb* initializeVCB(u_int64_t volumesize) {
    int number_of_blocks = (volumesize + BLOCKSIZE - 1) / BLOCKSIZE;
    vcb* volume = malloc(BLOCKSIZE);
    // Read from disk

    // initialize 
   volume->number_of_blocks = number_of_blocks;
   volume->size_of_block = BLOCKSIZE;
   volume->total_size_in_bytes = volume->number_of_blocks * volume->size_of_block;
   volume->magic_number = 0x12345678;
   volume->number_of_free_blocks = volume->number_of_blocks;
   // initialize free space
   volume->free_space_ptr = init_freeSpace(number_of_blocks, BLOCKSIZE);
   volume->LBA_free_space = 1;
   // initialize directory
   volume->LBA_root_directory = 13;

   // Write to disk
   return volume;

};
