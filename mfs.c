#include <string.h>
//#include "freeSpace.h"
#include "fsLow.h"
#include "mfs.h"

void __attribute__ ((constructor)) premain()
{   // THIS IS JUST FOR TEST 
       fs_init(&fdDIR);
}
int main(int argc, char * argv[]){
       printf("LBA is %llu\n", fdDIR.directoryStartLocation);
       return 0;
}

int fs_init(fdDir *DIR){
    int blockCount = (MIN_DE_NUM * sizeof(fs_directory_entry) + MINBLOCKSIZE) / MINBLOCKSIZE;
    int actualNumOfDE = (blockCount * MINBLOCKSIZE) / sizeof(fs_directory_entry);
    fs_directory_entry *directories =
        malloc(actualNumOfDE * sizeof(fs_directory_entry));
    // uint64_t directoryStartLocation = findMultipleBlocks(blockCount,vec);
    uint64_t directoryStartLocation = 64;
    for(unsigned int i = 0; i < actualNumOfDE; ++i){
        if(i == 0){
            strcpy((directories + i) -> dirEntryName, ".");
            (directories + i) -> dirEntryLocation = 0;
            (directories + i) -> dirParentLocation = 0;
        }else if(i == 1){
            strcpy((directories + i) -> dirEntryName, "..");
            (directories + i) -> dirEntryLocation = 0;
            (directories + i) -> dirParentLocation = 0;
        }else{
            strcpy((directories + i)
             -> dirEntryName, "undefined");
            (directories + i) -> dirEntryLocation = UNKNOWN_LOCATION;;
            (directories + i) -> dirParentLocation = UNKNOWN_LOCATION;
        }
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
        ((directories + i) -> metaData).fm_accessmode = 0063;
    }
    /*
    *  To-do: need to LBA Write
    */
    DIR -> directories = malloc(actualNumOfDE * sizeof(fs_directory_entry));
    DIR -> directoryStartLocation = directoryStartLocation;
    DIR -> dirEntryPosition = 0; 
    DIR -> directories = directories;
    DIR -> numberOfDir = actualNumOfDE;
    fs_display(DIR->directories,actualNumOfDE);
//    printf("test mkdir:\n");
//    fs_mkdir("root\\", 0777);
    return directoryStartLocation;
}

// Test function for display DE info
void fs_display(fs_directory_entry *pt, int numOfDE){
    printf("The content of DE is:\n");
    for(unsigned int i = 0; i < 3; ++i){
        printf("#%d DE:\n",i);
        printf("The entry name is %s\n", (pt + i)->dirEntryName);
        printf("Directory Location of current is %d\n", (pt + i)->dirEntryLocation);
        printf("Directory Location of parent is %d\n", (pt + i)->dirParentLocation);
        printf("entryType = %d\n", (int)((pt + i)->entryType));
        for(int j = 0; j < MIN_CHILD_NUM; ++j){
            printf("Child %d location is: %d\n", j ,((pt + i) -> childrenLocation)[j]);
        }
        printf("LBA of directory is:%llu\n", (pt + i) -> directoryStartLocation);
        printf("Meta data:\n");
        printf("The owner name is %s \n", ((pt + i) -> metaData).fm_ownername);
        printf("The group owner name is %s \n", ((pt + i) -> metaData).fm_groupownername);
        printf("size is %lld\n", ((pt + i) -> metaData).fm_size);
        printf("blocksize is %d", ((pt + i) -> metaData).fm_blksize);
        printf("# of blocks is %lld\n",((pt + i) -> metaData).fm_blocks);
        printf("The access time is: ");
        display_time(((pt + i) -> metaData).fm_accesstime);
        printf("\nThe modified time is: ");
        display_time(((pt + i) -> metaData).fm_modtime);
        printf("\nThe created time is: ");
        display_time(((pt + i) -> metaData).fm_createtime);
        printf("\n");
        print_accessmode(((pt + i) -> metaData).fm_accessmode, (int)((pt + i)->entryType));
        printf("\n");
    }
}
// End of test functions
/*===========================================================
* mkdir - create a new directory - Linux -> mkdir
============================================================*/
int fs_mkdir(const char *pathname, mode_t mode){
    // get the struct that hold all the DEs from LBA 'disl'
    /*
    ***To - Do ****
    1) Get the current path and split it to find the current 
       working directory.
    2) Located the DE of current working directory.
    3) Searching for the vacant child position and create the new DE
       for the new dir; if no vacant one then just extend thd length
       of the array of children pos of the current DE
    4) set up the attribute ofs new DE
    5) LBA write the new DE to the 'disk'
    */
    return 0;
}
/*===========================================================
* rmdir - remove a directory - Linux -> rmdir
============================================================*/
int fs_rmdir(const char *pathname){
    /*
    ***To - Do***
    1) Get the current path and split it to find the current
       working directory
    2) Located the DE of current working directory
    3) Check whether the giving dir name is existen in the children;
       if yes, check whether it is a directory and whether it is empty, if both yes them you can delete it, otherwise return an error or an exeception.
    */
   return 0;
}
/*===========================================================
* opendir - open a directory  - Linux -> cd
============================================================*/
fdDir * fs_opendir(const char *name){
    /*
    ***To - Do***
    1) Split the provided path and locate the current working directory.
    2) Located the DE of current working directory. If we can
    located it, set the current working directory(use another function) as provided. Otherwise just 
    */
   // code test start
   fdDir *cwdDir = malloc(sizeof(fdDIR));
   cwdDir -> directoryStartLocation = fdDIR.directoryStartLocation;
   cwdDir -> directories = fdDIR.directories;
   
   // code test end
   // the code section above should be done
   // by reading for LBA to construct the new directories
   return NULL;
}
/*===========================================================
* readdir - read a directory info - Linux -> no command but it
could use for ls
============================================================*/
struct fs_diriteminfo *fs_readdir(fdDir *dirp){
    /*
    ***To - Do***
    1) use fdDir to locate the DE.
    2) return the fs_diriteminfo struct generated from the DE.
    */
   return NULL;
}
/*===========================================================
* close dir - close an opened directory  - Linux -> quit
============================================================*/
int fs_closedir(fdDir *dirp){
    /*
    ***To - Do***
    1) closed dir ???? 
    */
   return 0;
}
/*===========================================================
* getcwd - get current working directory - Linux -> pwd
============================================================*/
char * fs_getcwd(char *buf, size_t size){
    /*
    ***To - Do***
    1) Located the DE for current working directory
    2) up-serach to the root to find the cwd in string and return it.
    */
   return 0;
}
/*===========================================================
* setcwd - change the current working directory - Linux -> chdir
============================================================*/
int fs_setcwd(char *buf){
    /*
    ***To - Do***
    1) Located the DE for the giving dir
    2) set the cwd as the giving dir
    */
   return 0;
}
/*===========================================================
* isFile - to validate the providing path is File or not 
============================================================*/
int fs_isFile(char * path){
    return 0;
    }//return 1 if file, 0 otherwise
/*===========================================================
* isDir - to validate the provding path is directory or not
============================================================*/
int fs_isDir(char * path){
    return 0;
}		//return 1 if directory, 0 otherwise
/*===========================================================
* delete - delete a file for the give dir
============================================================*/
int fs_delete(char* filename){
    return 0;
}	//removes a file

/*===========================================================
* Helper method
============================================================*/

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