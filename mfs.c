#include <string.h>
//#include "freeSpace.h"
#include "fsLow.h"
#include "mfs.h"
int fs_init(/* freeSpace* vec,char *volName */){
    int blockCount = (MIN_DE_NUM * sizeof(fs_directory_entry) + MINBLOCKSIZE) / MINBLOCKSIZE;
    int actualNumOfDE = (blockCount * MINBLOCKSIZE) / sizeof(fs_directory_entry);
    fs_directory_entry *directories =
        malloc(actualNumOfDE * sizeof(fs_directory_entry));
    // uint64_t directoryStartLocation = findMultipleBlocks(blockCount,vec);
    uint64_t directoryStartLocation = 64;
    for(unsigned int i = 0; i < actualNumOfDE; ++i){
        if(i == 0){
            strcpy((directories + i) -> dirEntryName, "root");
        }else{
            strcpy((directories + i) -> dirEntryName, "");
        }
        (directories + i) -> dirEntryLocation = i;
        (directories + i) -> dirParentLocation = i;
        for(int j = 0; j < MIN_CHILD_NUM; ++j){
            ((directories + i) -> childrenLocation)[j] = UNKNOWN_LOCATION;
        }
        (directories + i) -> entryType = FT_DIRECTORY;
        (directories + i) -> numberOfDirectories = actualNumOfDE;
        (directories + i) -> directoryStartLocation = directoryStartLocation;
        strcpy(((directories + i) -> metaData).fm_ownername, "staff");
        strcpy(((directories + i) -> metaData).fm_groupownername, "staff");
        ((directories + i) -> metaData).fm_size = blockCount * MINBLOCKSIZE;
        ((directories + i) -> metaData).fm_blksize = MINBLOCKSIZE;
        ((directories + i) -> metaData).fm_blocks = blockCount;
        ((directories + i) -> metaData).fm_accesstime = time(NULL);
        ((directories + i) -> metaData).fm_modtime = time(NULL);
        ((directories + i) -> metaData).fm_createtime = time(NULL);
        ((directories + i) -> metaData).fm_accessmode = 0x644;
    }
    /*
    *  To-do: need to LBA Write
    */
    free(directories);
    return directoryStartLocation;
    return 0;
}

int fs_mkdir(const char *pathname, mode_t mode){
    return 0;
}
int fs_rmdir(const char *pathname){
    return 0;
}
fdDir * fs_opendir(const char *name){
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
}   //linux chdir
int fs_isFile(char * path){
    return 0;
}	//return 1 if file, 0 otherwise
int fs_isDir(char * path){
    return 0;
}		//return 1 if directory, 0 otherwise
int fs_delete(char* filename){
    return 0;
}	//removes a file

int fs_stat(const char *path, struct fs_stat *buf){
    return 0;
}

void display_time(time_t t){
    // helper to display formatted time 
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

void print_accessmode(int access_mode, int file_type){
    // helper to display accessmod as "drwxrwxrwx" form
    char access_mode_str[11] = "----------";
    if(file_type == DT_DIR){
        access_mode_str[0] = 'd';
    }else if(file_type == DT_LNK){
        access_mode_str[0] = 'l';
    }
    int owner_access_mode = (0x700 & access_mode);
    if((owner_access_mode & 0x400) == 0x400){
        access_mode_str[1] = 'r';
    }
    if((owner_access_mode & 0x200) == 0x200){
        access_mode_str[2] = 'w';
    }
    if((owner_access_mode & 0x100) == 0x100){
        access_mode_str[3] = 'x';
    }
    int group_access_mode = (0x070 & access_mode);
    if((group_access_mode & 0x040) == 0x040){
        access_mode_str[4] = 'r';
    }
    if((group_access_mode & 0x020) == 0x020){
        access_mode_str[5] = 'w';
    }
    if((group_access_mode & 0x010) == 0x010){
        access_mode_str[6] = 'x';
    }
    int global_access_mode = (0x007 & access_mode);
    if((global_access_mode & 0x004) == 0x004){
        access_mode_str[7] = 'r';
    }
    if((global_access_mode & 0x002) == 0x002){
        access_mode_str[8] = 'w';
    }
    if((global_access_mode & 0x001) == 0x001){
        access_mode_str[9] = 'x';
    }
    printf("%s", access_mode_str);
}
