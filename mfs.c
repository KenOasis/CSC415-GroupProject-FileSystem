#include <string.h>
#include "freeSpace.h"
#include "fsLow.h"
#include "mfs.h"
#include "limits.h"
int fs_init(freeSpace* vec,char *volName){
 int blockCount = (MIN_DE_NUM * sizeof(fs_directory_entry) + MINBLOCKSIZE) / MINBLOCKSIZE;
    int actualNumOfDE = (blockCount * MINBLOCKSIZE) / sizeof(fs_directory_entry);
    fs_directory_entry *directories =
        malloc(actualNumOfDE * sizeof(fs_directory_entry));
    uint64_t directoryStartLocation = findMultipleBlocks(blockCount,vec);
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
        (directories + i) -> directoryStartLocation = directoryStartLocation;
        ((directories + i) -> metaData).fm_size = blockCount * MINBLOCKSIZE;
        ((directories + i) -> metaData).fm_blksize = MINBLOCKSIZE;
        ((directories + i) -> metaData).fm_blocks = blockCount;
        ((directories + i) -> metaData).fm_accesstime = time(NULL);
        ((directories + i) -> metaData).fm_modtime = time(NULL);
        ((directories + i) -> metaData).fm_createtime = time(NULL);
        ((directories + i) -> metaData).fm_accessmode = 0x721;
        /*
        *  To-do: need to LBA Write
        */
        free(directories);
        return directoryStartLocation;
    }
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

