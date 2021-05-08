mfs.c
    fs_int();
        (these are allocated)
        uint64_t LBA_inodes = 128
        uint64_t LBA_dir_ents = 512
        uint64_t LBA_root_directory = 64;
    fs_delete()
        // free  inode->fs_address (this is the LBA to free)

dir.c
    find_free_dir_ent() 
              /**To-Do**
         * extend more directory entries and more inodes, then get the new available space
         * remember to chage the directory info and write back to LBA
         */  
    getFileLBA()
          // ** To-do free the old LBA space
        // ** To-do get new allocate space for write
