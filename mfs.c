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
    fs_inode *inodes = malloc(MINBLOCKSIZE * blockCountInode);
    int blockCountDE = (actualNumInode * sizeof(fs_de) + MINBLOCKSIZE - 1) / MINBLOCKSIZE;
    /* the size of the inode is bigger than the size of DE, so we use the actual number of inodes to
    allocated the memory */
    fs_de *DEs = malloc(MINBLOCKSIZE * blockCountDE);

    // uint64_t LBA_inodes = findMultipleBlocks(blockCountInode,vector);
    uint64_t LBA_inodes = 128; // temperary for test
    //Initialize inodes
    for(int i = 0; i < actualNumInode; ++i){
        (inodes + i)->fs_size = 0;
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
            strcpy((DEs + i)->de_name, ".root");
            (DEs + i)->de_dotdot_inode = 0;
        }else{
            strcpy((DEs + i)->de_name, "undefined");
            (DEs + i)->de_dotdot_inode = UINT_MAX;
        }
        (DEs + i)->de_inode = i;
    }
    // uint64_t LBA_DEs = findMultipleBlocks(blockCountDE, vector);
    uint64_t LBA_DEs = 256; // for test only
    LBAwrite(DEs, blockCountDE, LBA_DEs);
    fs_directory *directory = malloc(sizeof(directory));

    directory->d_de_start_location = LBA_DEs;
    directory->d_inode_start_location = LBA_inodes;
    directory->d_num_inodes = actualNumInode;
    directory->d_num_DEs = actualNumInode;
    directory->d_inodes = inodes;
    directory->d_DEs = DEs;
    // uint64_t LBA_directory = findFreeBlock(vector);
    uint64_t LBA_directory = 64; // only for test
    Dir.Dir = malloc(sizeof(fs_directory));
    Dir.Dir = directory;
    LBAwrite(directory, 1, LBA_directory);
    free(inodes);
    free(DEs);
    // free(directory);
    return LBA_directory;
}

int fs_mkdir(const char *pathname, mode_t mode){
    return 0;
}

int fs_rmdir(const char *pathname){
   return 0;
}

fdDir * fs_opendir(const char *name){
   splitDIR *spdir = split_dir(name);
   //Restart from here!!!
   free_split_dir(spdir);
   return NULL;
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

splitDIR* split_dir(const char *name){
    const char delimiter[2] = "/";
    char *pathname;
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
        *(sDir->dir_names + i) = malloc(sizeof(char) * 32);
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