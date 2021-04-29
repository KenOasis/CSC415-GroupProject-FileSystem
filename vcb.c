
#include "vcb.h"
#include "mfs.h"
#include "dir.h"
#include "freeSpace.h"
int initializeVCB(uint64_t volumesize, uint64_t blocksize) {
    int number_of_blocks = (volumesize + BLOCKSIZE - 1) / BLOCKSIZE;
    vcb *v0 = malloc(BLOCKSIZE*2);
    // Read from disk
    // initialize
   v0 -> number_of_blocks = number_of_blocks;
   v0 -> size_of_block = BLOCKSIZE;
   v0 -> total_size_in_bytes = v0 -> number_of_blocks * v0 -> size_of_block;
   v0 -> magic_number = 0x12345678;
   v0 -> number_of_free_blocks = v0 -> number_of_blocks;
   // initialize free space
   v0 -> free_space_ptr = NULL; // todo
   v0 -> LBA_free_space = 1;

   // initialize directory

   v0 -> LBA_root_directory = fs_init();
   LBAwrite(v0, 1, 1);

    return 1;
};

vcb* bootVCB(uint64_t volumesize, uint64_t blocksize){
    vcb *vtemp = malloc(MINBLOCKSIZE);
    vcb *v0 = malloc(MINBLOCKSIZE);
	LBAread(vtemp, 1, 1);
	if(vtemp->magic_number != 0x12345678){
		printf("Initializing VCB......\n");
		initializeVCB(BLOCKSIZE*10, BLOCKSIZE);
	}
    LBAread(v0, 1, 1);
    strcpy(fs_DIR.cwd,"root/Users/");
    fs_DIR.LBA_root_directory = v0->LBA_root_directory;
    free(vtemp);
    vtemp = NULL;
    return v0;
}