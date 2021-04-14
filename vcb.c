typedef struct vcb {
    int number_of_blocks;  // number of blocks in volume
    int size_of_block;  // the size of a single block
    int total_size_in_bytes;  // the product of number_of_blocks and size_of_block
    int LBA_free_space;  // the LBA of free space
    int LBA_root_directory;  // the LBA of the root directory
    int magic_number;  // magic number: where to write what value?
    int number_of_free_blocks;  // number of free blocks
    freespace* free_space_ptr;  // not persistent
} vcb;
 

void initializeVCB(uinit64_t volumesize, uint64_t blocksize) {
    int number_of_blocks = (volumesize + blocksize - 1) / blocksize;
    *vcb = malloc(blocksize);
    // Read from disk

    // initialize 
   vcb.number_of_blocks = number_of_blocks;
   vcb.size_of_block = blocksize;
   vcb.total_size_in_bytes = vcb.number_of_blocks * vcb.size_of_block;
   vcb.magic_number = 0x12345678;
   vcb.number_of_free_blocks = vcb.number_of_blocks;
   // initialize free space
   initialize_free_space();
   // initialize directory
   initialize_directory();

   // Write to disk
   LBAwrite();

};