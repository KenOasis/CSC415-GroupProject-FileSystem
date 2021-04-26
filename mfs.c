#include <string.h>
//#include "freeSpace.h"
#include "fsLow.h"
#include "mfs.h"
#include "vcb.h"
// void __attribute__ ((constructor)) premain()
// {   // THIS IS JUST FOR TEST 
//        fs_init(&fdDIR);
// }
// int main(int argc, char * argv[]){
//        printf("LBA is %llu\n", fdDIR.directoryStartLocation);
//        return 0;
// }

uint64_t fs_init(/*freeSpace * vector*/){
    int blockCountInode = (MIN_DE_NUM * sizeof(fs_inode) + MINBLOCKSIZE - 1) / MINBLOCKSIZE;
    int actualNumInode = (blockCountInode * MINBLOCKSIZE) / sizeof(fs_inode);
    int blockCountDE = (actualNumInode * sizeof(fs_de) + MINBLOCKSIZE - 1) / MINBLOCKSIZE;
    /* the size of the inode is bigger than the size of DE, so we use the actual number of inodes to
    allocated the memory */
    fs_inode *inodes = malloc(blockCountInode * actualNumInode);

    fs_de *dir_ents = malloc(blockCountDE * actualNumInode);
    // uint64_t LBA_inodes = findMultipleBlocks(blockCountInode,vector);
    uint64_t LBA_inodes = 128; // temperary for test
    //Initialize inodes
    for(int i = 0; i < actualNumInode; ++i){
        (inodes + i)->fs_size = sizeof(fs_de);
        (inodes + i)->fs_blksize = 0;
        (inodes + i)->fs_blocks = 0;
        (inodes + i)->fs_accesstime = time(NULL);
        (inodes + i)->fs_modtime = time(NULL);
        (inodes + i)->fs_createtime = time(NULL);
        (inodes + i)->fs_entry_type = DT_DIR;
        (inodes + i)->fs_address = ULLONG_MAX;
        (inodes + i)->fs_accessmode = 0777;
    }
    LBAwrite(inodes,blockCountInode,LBA_inodes);
    //Initialize DEs
    for(int i = 0; i < actualNumInode; ++i){
        if( i == 0){
            strcpy((dir_ents + i)->de_name, "root");
            (dir_ents + i)->de_dotdot_inode = 0;
        }else if(i == 1){
            strcpy((dir_ents + i)->de_name, "Users");
            (dir_ents + i)->de_dotdot_inode = 0;
        }else if(i == 2){
            strcpy((dir_ents + i)->de_name, "Admin");
            (dir_ents + i)->de_dotdot_inode = 1;
        }else if(i == 3){
            strcpy((dir_ents + i)->de_name, "Guest");
            (dir_ents + i)->de_dotdot_inode = 1;
        }else if(i == 4){
            strcpy((dir_ents + i)->de_name, "Jimmy");
            (dir_ents + i)->de_dotdot_inode = 1;
        }else{
            strcpy((dir_ents + i)->de_name, "uninitialized");
            (dir_ents + i)->de_dotdot_inode = UINT_MAX;
        }
        (dir_ents + i)->de_inode = i;
    }
    // uint64_t LBA_DEs = findMultipleBlocks(blockCountDE, vector);
    uint64_t LBA_dir_ents = 512; // for test only

    LBAwrite(dir_ents, blockCountDE, LBA_dir_ents);

    printf("actual number inodes = %d\n", actualNumInode);
    fs_directory *directory = (fs_directory*)malloc(sizeof(fs_directory));
    directory->d_de_start_location = LBA_dir_ents;
    directory->d_de_blocks = blockCountDE;
    directory->d_inode_start_location = LBA_inodes;
    directory->d_inode_blocks = blockCountInode;
    directory->d_num_inodes = actualNumInode;
    directory->d_num_DEs = actualNumInode;
    // uint64_t LBA_root_directory = findFreeBlock(vector);
    uint64_t LBA_root_directory = 64; // only for test
    // (fs_directory*)(fs_DIR.Dir) = malloc(sizeof(fs_directory));
    // fs_DIR.Dir = directory;
    LBAwrite(directory, 2, LBA_root_directory);
    fs_DIR.LBA_root_directory = LBA_root_directory;
    free(inodes);
    free(dir_ents);
    free(directory);
    return LBA_root_directory;
}

int fs_mkdir(const char *pathname, mode_t mode){
    return 0;
}

int fs_rmdir(const char *pathname){
   return 0;
}

fdDir * fs_opendir(const char *name){
   splitDIR *spdir = split_dir(name);
   fdDir *dirp;
    dirp = malloc(sizeof(fdDir));
    uint32_t de_pos = find_DE_pos(spdir);
   if(de_pos == UINT32_MAX){
       char *filename = *(spdir->dir_names + (spdir->length - 1));
       printf("no such file or direcotry: %s\n", filename);
       free_split_dir(spdir);
       return NULL;
   }
   printf("pos is %u\n", de_pos);
   uint32_t num_children = 8;
   blkcnt_t children_blocks = (num_children * sizeof(fs_de) + MINBLOCKSIZE - 1) / MINBLOCKSIZE;
   dirp->childrens = malloc(MINBLOCKSIZE * children_blocks);
   dirp->dirEntryPosition = de_pos;
   find_childrens(dirp);
   free_split_dir(spdir);
   return dirp;
}

struct fs_diriteminfo *fs_readdir(fdDir *dirp){

   return NULL;
}
int fs_closedir(fdDir *dirp){
   return 0;
}

char * fs_getcwd(char *buf, size_t size){
   return 0;
}

int fs_setcwd(char *buf){

   return 0;
}

int fs_isFile(char * path){
    return 0;
    }//return 1 if file, 0 otherwise

int fs_isDir(char * path){
    return 0;
}		//return 1 if directory, 0 otherwise

int fs_delete(char* filename){
    return 0;
}	//removes a file

//Helper
int reload_directory(fs_directory * directory){
    //reload Dir_info from Disk(LBA)
    directory->d_inodes = malloc(MINBLOCKSIZE * (directory->d_inode_blocks));
    directory->d_dir_ents = malloc(MINBLOCKSIZE * (directory->d_de_blocks));
    LBAread(directory->d_inodes, directory->d_inode_blocks, directory->d_inode_start_location);
    LBAread(directory->d_dir_ents, directory->d_de_blocks, directory->d_de_start_location);
    //End of reload Dir_info
    return 0;
}
// @return value: UINT32_MAX means failed to find pos.
void free_directory(fs_directory *directory){
    free(directory->d_dir_ents);
    free(directory->d_inodes);
    free(directory);
}
uint32_t find_DE_pos(splitDIR *spdir){
    fs_directory* directory = malloc(MINBLOCKSIZE);
	LBAread(directory, 1, fs_DIR.LBA_root_directory);
	reload_directory(directory);
    int found_dir_count = 0;
    int parent_pos = 0;
    int current_parent_pos = 0;
    uint32_t pos = 0;
    for(int i = 0; i < (spdir->length); ++i){
        for(int j = 0; j < (directory->d_num_DEs); ++j){
            int not_equal_names = strcmp(*(spdir->dir_names + i),(directory->d_dir_ents + j)->de_name);
            current_parent_pos = (directory->d_dir_ents + j)->de_dotdot_inode;
            int is_child = (current_parent_pos == parent_pos);
            // printf("current parrent position: %d\n", current_parent_pos);
            // printf("parent position is %d\n", parent_pos);
            // printf("is chlid: %d\n", is_child);
            // printf("name equal: %d\n", !not_equal_names);
            if((!not_equal_names) && is_child){
                found_dir_count++;
                pos = (directory->d_dir_ents + j) ->de_inode;
                parent_pos = pos;
                break;
            }
        }
        if(found_dir_count < i){
                break;
        }
    }
    if(found_dir_count == spdir->length){
        return pos;
    }else{
        return UINT32_MAX;
    }
}

int find_childrens(fdDir *dirp){
    // uint32_t parent_inode = dirp->dirEntryPosition;
    // fs_directory* directory = (fs_directory*)fs_DIR.Dir;
    // int count_children = 0;
    // if(parent_inode != UINT32_MAX){
    //     for(int i = 0; i < directory->d_num_DEs; ++i){
    //         if((directory->d_DEs + i)->de_dotdot_inode == parent_inode){
    //             count_children++;
    //         }
    //     }
    // }
    // if(count_children == 0){
    //     return 0;
    // }else{
    //     dirp->childrens = (fs_de**)malloc(sizeof(fs_de*)*count_children);
    //     for(int i = 0; i < count_children; ++i){
    //         *(dirp->childrens + i) = (fs_de*)malloc(sizeof(fs_de));
    //     }
    //     dirp->num_children = count_children;
    //     int pos = 0;
    //     for(int i =0; i < directory->d_num_DEs; ++i){
    //         if((directory->d_DEs + i)->de_dotdot_inode == parent_inode){
    //             *(dirp->childrens + pos) = (directory->d_DEs + i);
    //             count_children--;
    //         }
    //         if(count_children == 0){
    //             break;
    //         }
    //     }
    //     return 1;
    // }
    return 0;
}
splitDIR* split_dir(const char *name){
    const char delimiter[2] = "/";
    char *pathname = malloc(sizeof(char)*256);
    char *token;
    int length = 0;
    strcpy(pathname, name);
    token = strtok(pathname, delimiter);
    while(token != NULL){
        length++;
        token = strtok(NULL, delimiter);
    }
    splitDIR *sDir = malloc(sizeof(splitDIR));
    sDir->dir_names = (char**)malloc(sizeof(char *) * length);
    sDir->length = length;
    for(int i = 0; i < length; ++i){
        *(sDir->dir_names + i) = malloc(sizeof(char) * 64);
    }
    // remember to free outside the function
    strcpy(pathname, name);
    token = strtok(pathname, delimiter);
    int i = 0;
    while ((token != NULL) && (strcmp(token, "") != 0)) 
    {
        strcpy(*(sDir->dir_names + i), token);
        i++;
        token = strtok(NULL, delimiter);
    }
    free(pathname);
    return sDir;    
}

void free_split_dir(splitDIR *spdir){
    for(int i = 0; i < spdir->length; ++i){
        free(*(spdir->dir_names + i));
    }
    free(spdir);
}
void display_time(time_t t){
    
    if (t == -1) {
        
        puts("The time() function failed");
        exit(EXIT_FAILURE);
    }
    
    struct tm *ptm = localtime(&t);
    
    if (ptm == NULL) {
        
        puts("The localtime() function failed");
        exit(EXIT_FAILURE);
    }
    char *month_str[12] = {
        "Jan",
        "Feb",
        "Mar",
        "Apr",
        "May",
        "Jun",
        "Jul",
        "Aug",
        "Sep",
        "Oct",
        "Nov",
        "Dec"
    };
    printf("%s %02d %02d:%02d\n", month_str[ptm->tm_mon],ptm->tm_mday, ptm->tm_hour, 
           ptm->tm_min);
}

void print_accessmode(int access_mode, int file_type){    char access_mode_str[11] = "----------";
    if(file_type == DT_DIR){
        access_mode_str[0] = 'd';
    }else if(file_type == DT_LNK){
        access_mode_str[0] = 'l';
    }
    int owner_access_mode = (0700 & access_mode);
    if((owner_access_mode & 0400) == 0400){
        access_mode_str[1] = 'r';
    }
    if((owner_access_mode & 0200) == 0200){
        access_mode_str[2] = 'w';
    }
    if((owner_access_mode & 0100) == 0100){
        access_mode_str[3] = 'x';
    }
    int group_access_mode = (0070 & access_mode);
    if((group_access_mode & 0040) == 0040){
        access_mode_str[4] = 'r';
    }
    if((group_access_mode & 0020) == 0020){
        access_mode_str[5] = 'w';
    }
    if((group_access_mode & 0010) == 0010){
        access_mode_str[6] = 'x';
    }
    int global_access_mode = (0007 & access_mode);
    if((global_access_mode & 0004) == 0004){
        access_mode_str[7] = 'r';
    }
    if((global_access_mode & 0002) == 0002){
        access_mode_str[8] = 'w';
    }
    if((global_access_mode & 0001) == 0001){
        access_mode_str[9] = 'x';
    }
    printf("%s", access_mode_str);
}