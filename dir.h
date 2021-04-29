#ifndef _DIR_H
#define _DIR_H
#include <stdlib.h>
#include <stdio.h>
#include <sys/types.h>
#include <unistd.h>
#include <time.h>
#include <limits.h>
#include <string.h>
#include <dirent.h>
#include "fsLow.h"
#include "mfs.h"
#include "freeSpace.h"



struct fs_inode{
	off_t     fs_size;    		/* total size, in bytes */
	blksize_t fs_blksize; 		/* blocksize for file system I/O */
	blkcnt_t  fs_blocks;  		/* number of 512B blocks allocated */
	time_t    fs_accesstime;   	/* time of last access */
	time_t    fs_modtime;   	/* time of last modification */
	time_t    fs_createtime;   	/* time of last status change */
	unsigned char fs_entry_type; /* entry type , file or directory*/
	uint64_t  fs_address;  /* inode address to find the actual data */
  int       fs_accessmode;      /* access mode */
};
typedef struct fs_inode fs_inode;
struct fs_de{
	char de_name[256];
	uint32_t de_inode; /* inode number of current directory */
	uint32_t de_dotdot_inode; /* inode number of parent direcotry */
};
typedef struct fs_de fs_de;

struct fs_directory{
	uint64_t d_de_start_location;		/* Starting LBA of directory entries */ 
	blkcnt_t d_de_blocks; /* number of blocks for directory entries */
	uint64_t d_inode_start_location; /* Starting 
	LBA of the inodes */
	blkcnt_t d_inode_blocks; /* number of blocks for inodes */
	uint32_t d_num_inodes; /* number of inodes */
	uint32_t d_num_DEs; /* number of directory entry */
	fs_inode * d_inodes; /* inodes, not pertain */
	fs_de * d_dir_ents; /* DEs, not pertain */
};
typedef struct fs_directory fs_directory;

typedef struct{
	 char cwd[4096];
	 uint64_t LBA_root_directory;
}DirInfo;

struct splitDIR{
	int length;
	char **dir_names;
};
typedef struct splitDIR splitDIR;

typedef struct fdDir fdDir;

uint32_t find_free_dir_ent(fs_directory *directory);
int reload_directory(fs_directory *directory);
int write_direcotry(fs_directory *directory);
void free_directory(fs_directory* directory);
uint32_t find_DE_pos(splitDIR *spdir);
int check_duplicated_dir(uint32_t parent_de_pos, char* name);
int find_childrens(fdDir *dirp);
splitDIR* split_dir(const char *name);
void free_split_dir(splitDIR *spdir);
void display_time(time_t t); // helper to display formatted time 
void print_accessmode(int access_mode, int file_type); // helper to display accessmod as "drwxrwxrwx" form
struct fs_diriteminfo* getDirInfo();

DirInfo fs_DIR;

#endif