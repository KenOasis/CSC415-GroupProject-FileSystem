#include <vcb.h>

void initializeVCB(uinit64_t volumesize, uint64_t blocksize) {
    int number_of_blocks = (volumesize + BLOCKSIZE - 1) / BLOCKSIZE;
    *vcb = malloc(BLOCKSIZE);
    // Read from disk

    // initialize 
   vcb.number_of_blocks = number_of_blocks;
   vcb.size_of_block = BLOCKSIZE;
   vcb.total_size_in_bytes = vcb.number_of_blocks * vcb.size_of_block;
   vcb.magic_number = 0x12345678;
   vcb.number_of_free_blocks = vcb.number_of_blocks;
   // initialize free space
   vcb.free_space_ptr = initialize_free_space();
   vcb.LBA_free_space = initialize_free_space();
   // initialize directory
   vcb.LBA_root_directory = initialize_directory();

   // Write to disk


};