/**************************************************************
* Class:  CSC-415
* Name: Professor Bierman
* Student ID: N/A
* Project: Basic File System
*
* File: fsLow.h
*
* Description: 
*	This is the file system interface.
*	This is the interface needed by the driver to interact with
*	your filesystem.
*
**************************************************************/
#ifndef _MFS_H
#define _MFS_H
#include <stdlib.h>
#include <stdio.h>
#include <sys/types.h>
#include <unistd.h>
#include <time.h>
#include <limits.h>
#include "b_io.h"
//#include "freeSpace.h"
#include "fsLow.h"
#include <dirent.h>
#define FT_REGFILE	DT_REG
#define FT_DIRECTORY DT_DIR
#define FT_LINK	DT_LNK
#define MIN_DE_NUM 16
#define MIN_CHILD_NUM 16
#define UNKNOWN_LOCATION USHRT_MAX
#ifndef uint64_t
typedef u_int64_t uint64_t;
#endif
#ifndef uint32_t
typedef u_int32_t uint32_t;
#endif


typedef struct{
    char      fm_ownername[32];     /* ownername of the file */
    char      fm_groupownername[32];/* group ownername of the file */
	off_t     fm_size;    		/* total size, in bytes */
	blksize_t fm_blksize; 		/* blocksize for file system I/O */
	blkcnt_t  fm_blocks;  		/* number of 512B blocks allocated */
	time_t    fm_accesstime;   	/* time of last access */
	time_t    fm_modtime;   	/* time of last modification */
	time_t    fm_createtime;   	/* time of last status change */
    int       fm_accessmode;      /* access mode */
}fs_metadata;

typedef struct{
	char dirEntryName[256];
	unsigned short dirEntryLocation; /* Current directory entry position */
	unsigned short dirParentLocation; /* Parent directory entry position */
	unsigned short childrenLocation[MIN_CHILD_NUM]; /* Location Of Children Entry, if entryType is file then it is all -1*/
    unsigned short numberOfDirectories; /* Total number of directory */
	unsigned char entryType; /* file or directory*/
	uint64_t directoryStartLocation; /*Starting LBA of directory */
	fs_metadata metaData; // file attributes
}fs_directory_entry;

struct fs_diriteminfo
	{
    unsigned short d_reclen;    /* length of this record */
    unsigned char fileType;    
    char d_name[256]; 			/* filename max filename is 255 characters */
	};


typedef struct
	{
	/*****TO DO:  Fill in this structure with what your open/read directory needs  *****/
	unsigned short  d_reclen;		/*length of this record */
	unsigned short	dirEntryPosition;	/*which directory entry position, like file pos */
	uint64_t	directoryStartLocation;		/*Starting LBA of directory */
    fs_directory_entry *directories; 
	} fdDir;

// Test funcitions
void fs_display(fs_directory_entry *pt, int numOfDE);
// End of test functions
int fs_init(fdDir *DIR);
int fs_mkdir(const char *pathname, mode_t mode);
int fs_rmdir(const char *pathname);
fdDir * fs_opendir(const char *name);
struct fs_diriteminfo *fs_readdir(fdDir *dirp);
int fs_closedir(fdDir *dirp);

char * fs_getcwd(char *buf, size_t size);
int fs_setcwd(char *buf);   //linux chdirs
int fs_isFile(char * path);	//return 1 if file, 0 otherwise
int fs_isDir(char * path);		//return 1 if directory, 0 otherwise
int fs_delete(char* filename);	//removes a file

struct fs_stat
	{
    char      st_ownername[32];     /* ownername of the file */
    char      st_groupownername[32];/* group ownername of the file */
	off_t     st_size;    		/* total size, in bytes */
	blksize_t st_blksize; 		/* blocksize for file system I/O */
	blkcnt_t  st_blocks;  		/* number of 512B blocks allocated */
	time_t    st_accesstime;   	/* time of last access */
	time_t    st_modtime;   	/* time of last modification */
	time_t    st_createtime;   	/* time of last status change */
	int       access_mode;      /* access mode */
	/* add additional attributes here for your file system */
	};

int fs_stat(const char *path, struct fs_stat *buf);

void display_time(time_t t); // helper to display formatted time 
void print_accessmode(int access_mode, int file_type); // helper to display accessmod as "drwxrwxrwx" form
#endif
