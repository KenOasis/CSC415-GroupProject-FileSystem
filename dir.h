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
#include <fcntl.h>
#include "fsLow.h"
#include "freeSpace.h"
#ifndef DIR_MAXLENGTH
#define DIR_MAXLENGTH 4096
#endif

/****************************************************
*  struct type to hold inodes which contains the meta
*  date of the file or directory
****************************************************/

typedef struct{
	off_t     fs_size;    		/* total size, in bytes */
	blksize_t fs_blksize; 		/* blocksize for file system I/O */
	blkcnt_t  fs_blocks;  		/* number of 512B blocks allocated */
	time_t    fs_accesstime;   	/* time of last access */
	time_t    fs_modtime;   	/* time of last modification */
	time_t    fs_createtime;   	/* time of last status change */
	unsigned char fs_entry_type; /* entry type , file or directory*/
	uint64_t  fs_address;  /* inode address to find the actual data */
  int       fs_accessmode;      /* access mode */
}fs_inode;


/****************************************************
* struct type to hold the directory entry info
* this struct maintain the logical internal structure
* (tree) of the directory/file system
****************************************************/
typedef struct{
	char de_name[256];
	uint32_t de_inode; /* inode number of current directory */
	uint32_t de_dotdot_inode; /* inode number of parent direcotry */
}fs_de;

/****************************************************
* struct type to hold the directory info. It is use 
* for locating and accessing the inodes and directory 
* entries. 
****************************************************/
typedef struct {
	uint64_t d_de_start_location;		/* Starting LBA of directory entries */ 
	blkcnt_t d_de_blocks; /* number of blocks for directory entries */
	uint64_t d_inode_start_location; /* Starting 
	LBA of the inodes */
	blkcnt_t d_inode_blocks; /* number of blocks for inodes */
	uint32_t d_num_inodes; /* number of inodes */
	uint32_t d_num_DEs; /* number of directory entry */
	fs_inode * d_inodes; /* inodes, not pertain */
	fs_de * d_dir_ents; /* DEs, not pertain */
}fs_directory;


/****************************************************
struct to hold the global varibale with LBA root
directory and the cwd
****************************************************/
typedef struct{
	 char cwd[DIR_MAXLENGTH + 1];
	 uint64_t LBA_root_directory;
}DirInfo;

/****************************************************
struct to hold splited name of each level of dir name
****************************************************/
typedef struct{
	int length;
	char **dir_names;
}splitDIR;

typedef struct{
	int capacity;
	int top; // point to the postion after the top
	char **strings;
}stringStack;


/****************************************************
* @parameters 
*   @type fs_directory*: directory 
* @return
*   @type uint_32: a number represent the position
*                  of the directory entry
* This function return the position of the next free
* directory entry.
****************************************************/
uint32_t find_free_dir_ent(fs_directory *directory);

/****************************************************
* @parameters 
*   @type fs_directory*: directory
* @return
*   @type int: 0 is success, 1 is fail
* This function reload the directory from LBA space
****************************************************/
int reload_directory(fs_directory *directory);

/****************************************************
* @parameters 
*   @type fs_directory*: directory
* @return
*   @void
*  Destructor of the directory to free the memory
*  of the struct type directory
****************************************************/
int write_directory(fs_directory *directory);

/****************************************************
* @parameters 
*   @type fs_directory*: directory
* @return
*   @type int: 0 is success, 1 is fail
* This function write directory back to LBA
****************************************************/
void free_directory(fs_directory* directory);

/****************************************************
* @parameters 
*   @type splitDIR*: a struct hold splited path info
* @return
*   @type uint_32: a number represent the position
*                  of the directory entry, If it equal
                   UINT32_MAX, it means fail
* This function return the position of the directory
* entry. 
****************************************************/
uint32_t find_DE_pos(splitDIR *spdir);

/****************************************************
* @parameters 
*   @type uint32_t: position of the parent directory
*                   entry
*   @type char*: name of the new directory (to be addded)
* @return
*   @type int: 1 is duplicate, 0 is not
* This function check whether the new dir has duplicated
* name in the parent directory
****************************************************/
int is_duplicated_dir(uint32_t parent_de_pos, const char* name);

/****************************************************
* @parameters 
*   @type const char*: path to be splited 
* @return
*   @type splitDIR*: a pointer point to a struct splitDIR
*                    which hold the splited path
* This function split the given path into seperate names
* of each level of directory/file
****************************************************/
splitDIR* split_dir(const char *name);

/****************************************************
* @parameters 
*   @type splitDIR*: the splited path info
* @return
*   @type void
* This destructor to free the allocated memory of 
* strut type splitDIR
****************************************************/
void free_split_dir(splitDIR *spdir);



char *get_absolute_path(char* argv);

int is_File(char *fullpath);
int is_Dir(char *fullpath);
uint64_t getFileLBA(const char *filename, int flags);

blkcnt_t getBlocks(const char *filename);

off_t getFileSize(const char *filename);

int setFileSize(const char *filename, off_t filesize);

int setFileBlocks(const char *filename, blkcnt_t count);

int setFileLBA(const char *filename, uint64_t Address);

stringStack* initStack(int capacity);

int pushIntoStack(stringStack* stack, char* string);

char* popFromStack(stringStack* stack);

int free_stack(stringStack* stack);
int updateAccessTime(uint32_t inode);

int updateModTime(uint32_t inode);

/****************************************************
*  helper function to format time output, test only
****************************************************/
char* display_time(time_t t); 

/****************************************************
*  helper function to format accessmode output, test only
****************************************************/
void print_accessmode(int access_mode, int file_type); 

DirInfo fs_DIR;
#endif