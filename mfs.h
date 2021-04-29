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
#include <string.h>
#include <dirent.h>
#include "b_io.h"
#include "freeSpace.h"
#include "fsLow.h"
#include "fs_dir.h"
#define FT_REGFILE	DT_REG
#define FT_DIRECTORY DT_DIR
#define FT_LINK	DT_LNK
#define MIN_DE_NUM 128
#define UNKNOWN_LOCATION USHRT_MAX
#ifndef uint64_t
typedef u_int64_t uint64_t;
#endif
#ifndef uint32_t
typedef u_int32_t uint32_t;
#endif


// typedef struct{
// 	off_t     fs_size;    		/* total size, in bytes */
// 	blksize_t fs_blksize; 		/* blocksize for file system I/O */
// 	blkcnt_t  fs_blocks;  		/* number of 512B blocks allocated */
// 	time_t    fs_accesstime;   	/* time of last access */
// 	time_t    fs_modtime;   	/* time of last modification */
// 	time_t    fs_createtime;   	/* time of last status change */
// 	unsigned char fs_entry_type; /* entry type , file or directory*/
// 	uint64_t  fs_address;  /* inode address to find the actual data */
//   int       fs_accessmode;      /* access mode */
// }fs_inode;

// typedef struct{
// 	char de_name[256];
// 	uint32_t de_inode; /* inode number of current directory */
// 	uint32_t de_dotdot_inode; /* inode number of parent direcotry */
// }fs_de;


// typedef struct{
// 	uint64_t d_de_start_location;		/* Starting LBA of directory entries */ 
// 	blkcnt_t d_de_blocks; /* number of blocks for directory entries */
// 	uint64_t d_inode_start_location; /* Starting 
// 	LBA of the inodes */
// 	blkcnt_t d_inode_blocks; /* number of blocks for inodes */
// 	uint32_t d_num_inodes; /* number of inodes */
// 	uint32_t d_num_DEs; /* number of directory entry */
// 	fs_inode * d_inodes; /* inodes, not pertain */
// 	fs_de * d_dir_ents; /* DEs, not pertain */
// }fs_directory;
typedef struct fs_de fs_de;

struct fs_diriteminfo
	{
		unsigned short d_reclen;    /* length of this record Not used */
    unsigned char file_type;    
    char d_name[256]; 			/* filename max filename is 255 characters */
	};


struct fdDir
	{
	/*****TO DO:  Fill in this structure with what your open/read directory needs  *****/
	uint32_t	de_pos;	/*which directory entry position, like file pos */
	uint32_t cur_pos;
	uint32_t num_children;
	fs_de **childrens;
	};

typedef struct fdDir fdDir;
// typedef struct{
// 	 char cwd[4096];
// 	 uint64_t LBA_root_directory;
// }DirInfo;

// typedef struct{
// 	int length;
// 	char **dir_names;
// }splitDIR;


uint64_t fs_init(/*freeSpace * vector*/);
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
	off_t     st_size;    		/* total size, in bytes */
	blksize_t st_blksize; 		/* blocksize for file system I/O */
	blkcnt_t  st_blocks;  		/* number of 512B blocks allocated */
	time_t    st_accesstime;   	/* time of last access */
	time_t    st_modtime;   	/* time of last modification */
	time_t    st_createtime;   	/* time of last status change */
	int       st_accessmode;      /* access mode */
	uint64_t  st_file_address; /* LBA address of file */
	/* add additional attributes here for your file system */ 
	};

int fs_stat(const char *path, struct fs_stat *buf);

// uint32_t find_free_dir_ent(fs_directory *directory);
// int reload_directory(fs_directory *directory);
// int write_direcotry(fs_directory *directory);
// void free_directory(fs_directory* directory);
// uint32_t find_DE_pos(splitDIR *spdir);
// int check_duplicated_dir(uint32_t parent_de_pos, char* name);
// int find_childrens(fdDir *dirp);
// splitDIR* split_dir(const char *name);
// void free_split_dir(splitDIR *spdir);
// void display_time(time_t t); // helper to display formatted time 
// void print_accessmode(int access_mode, int file_type); // helper to display accessmod as "drwxrwxrwx" form

// DirInfo fs_DIR;

// // remember to free the pointer at end of main

#endif
