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
#include "dir.h"
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

#ifndef DIR_MAXLENGTH
#define DIR_MAXLENGTH 4096
#endif

/****************************************************
*  struct type to hold dir info of the cureent cwd
****************************************************/
struct fs_diriteminfo
	{
		unsigned short d_reclen;    /* length of this record not used*/
    unsigned char file_type;    
    char d_name[256]; 			/* filename max filename is 255 characters */
	};

/****************************************************
*  struct type to hold the current directory info
*  children means that under the current cwd
*  each of the sub direcotry entries
****************************************************/
typedef struct {
	/*****TO DO:  Fill in this structure with what your open/read directory needs  *****/
	unsigned short d_reclen;    /* length of this record not used*/
	unsigned short dirEntryPosition; /* not used*/
	uint32_t de_pos;	/* which directory entry position, like file pos */
	uint32_t cur_pos; /* current pos (cursor) of the iterated children */
	uint32_t num_children; /* total number of children of this dir */
	struct fs_diriteminfo **children; /* pointer to hold children info */
	}fdDir;

/****************************************************
* @parameters 
*   @type uint_32: a number represent the position
*                  of the directory entry
* @return
*   @type uint64_int: LBA address of directory
* This function return the position of the next free
* directory entry.
* it has a initial direcoty as:
*                    root
*                      |
*                    Users
*                      |
*            Admin   Guest  Jimmy
*
*   the init cwd is root/Users
****************************************************/
uint64_t fs_init();

/****************************************************
* @parameters 
*   @type const char* : the file name to add as dir
*		@type mode_t: a 3digit oct number represent the 
*									access mode ie. 0777;
* @return
*   @type int: 1 means succes, 0 means fail
* This function create a new direcotry 
****************************************************/
int fs_mkdir(const char *pathname, mode_t mode);

/****************************************************
* @parameters 
*   @type const char* : the dir name to be removed
* This function remove a directory (if exist)
* @return
*   @type int: 1 means succes, 0 means fail
* !Need to validte whether is direcotry before execute
****************************************************/
int fs_rmdir(const char *pathname);

/****************************************************
* @parameters 
*   @type const char* : the dir (fullpath) need to be 
*												opened and iterated.
* @return
*   @type fdDir* : a struct hold the current direcoty
*									info to iterate the children
* This function opens a dir(fullpath) to iterate the
*	children of them
****************************************************/
fdDir * fs_opendir(const char *name);

/****************************************************
* @parameters 
*   @type fdDir* : the struct hold current dir-
*			-ectory info to iterate the children 
* @return
*   @type struct dirinteminfo* : a struct hold the 
*			children info includes name and type;
* This function is used to itreate the children of 
*	the given struct(fdDir) for a direcotry
****************************************************/
struct fs_diriteminfo *fs_readdir(fdDir *dirp);

/****************************************************
* @parameters 
*   @type fdDir* : the direcotry struct fdDir to close
* @return
*   @type int: 1 is success, 0 is fail
* This function is used to close and clean the fdDir
* object and do something to close the itreation
****************************************************/
int fs_closedir(fdDir *dirp);

/****************************************************
* @parameters 
*   @type char*: buf to store the cwd info
@		@type size_t: the size of given buf;
* @return
*   @type char*: the cwd info (not the buf);
* This function is used to get the current working
*	directory.
****************************************************/
char * fs_getcwd(char *buf, size_t size);

/****************************************************
* @parameters 
*   @type char*: the path info to change cwd
* @return
*   @type int : 1 is success, 0 is fail;
* This function is used to change the current working
* directory
****************************************************/
int fs_setcwd(char *buf);   //linux chdirs

/****************************************************
* @parameters 
*   @type char*: the name of file to check whether it
*								is the File type
* @return
*   @type int: 1 is for File, 0 otherwise
* This function is used to check whether a given file
* name is the type File or not
****************************************************/
int fs_isFile(char * path);	//return 1 if file, 0 otherwise

/****************************************************
* @parameters 
*   @type char*: the name of file to check whether it
*								is the Dir type
* @return
*   @type int: 1 is for Dir, 0 otherwise
* This function is used to check whether a given file
* name is the type Dir or not
****************************************************/
int fs_isDir(char * path);		//return 1 if directory, 0 otherwise

/****************************************************
* @parameters 
*   @type char*: the name of file to delete
* @return
*   @type int: 0 is success, 1 is fail
* This function is used to delete a File
* !Need to verify whether a given filename is File
* before calling this function to delete
****************************************************/
int fs_delete(char* filename);	//removes a file

/****************************************************
*  struct type to hold the current file info which is
* use for the ls command to iterate the info of file
****************************************************/
struct fs_stat
	{
	off_t     st_size;    		/* total size, in bytes */
	blksize_t st_blksize; 		/* blocksize for file system I/O */
	blkcnt_t  st_blocks;  		/* number of 512B blocks allocated */
	time_t    st_accesstime;   	/* time of last access */
	time_t    st_modtime;   	/* time of last modification */
	time_t    st_createtime;   	/* time of last status change */
	int       st_accessmode;      /* access mode */
	/* add additional attributes here for your file system */ 
	};

/****************************************************
* @parameters 
*   @type char*: the name of file to get info for
*		@type struct fs_stat*: fs_stat struct to hold
8													file info.
* @return
*   @type int: 1 is success, 0 is fail
* This function is used for the ls command to 
* iterate the info of each file(children)
****************************************************/
int fs_stat(const char *path, struct fs_stat *buf);

#endif
