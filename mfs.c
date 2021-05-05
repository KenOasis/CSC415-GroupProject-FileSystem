
#include <string.h>
#include "freeSpace.h"
#include "fsLow.h"
#include "mfs.h"
#include "vcb.h"
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
        if(i < 5){
            (inodes + i)->fs_entry_type = DT_DIR;
        }else{
            (inodes + i)->fs_entry_type = DT_REG;
        }
        (inodes + i)->fs_size = sizeof(fs_de);
        (inodes + i)->fs_blksize = 0;
        (inodes + i)->fs_blocks = 0;
        (inodes + i)->fs_accesstime = time(NULL);
        (inodes + i)->fs_modtime = time(NULL);
        (inodes + i)->fs_createtime = time(NULL);
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
            (dir_ents + i)->de_dotdot_inode = UINT32_MAX;
        }
        (dir_ents + i)->de_inode = i;
    }
    // uint64_t LBA_DEs = findMultipleBlocks(blockCountDE, vector);
    uint64_t LBA_dir_ents = 512; // for test only

    LBAwrite(dir_ents, blockCountDE, LBA_dir_ents);

    fs_directory *directory = (fs_directory*)malloc(sizeof(fs_directory));
    directory->d_de_start_location = LBA_dir_ents;
    directory->d_de_blocks = blockCountDE;
    directory->d_inode_start_location = LBA_inodes;
    directory->d_inode_blocks = blockCountInode;
    directory->d_num_inodes = actualNumInode;
    directory->d_num_DEs = actualNumInode;
    // uint64_t LBA_root_directory = findFreeBlock(vector);
    uint64_t LBA_root_directory = 64; // only for test
    LBAwrite(directory, 1, LBA_root_directory);
    fs_DIR.LBA_root_directory = LBA_root_directory;
    free(inodes);
    free(dir_ents);
    free(directory);
    inodes = NULL;
    dir_ents = NULL;
    directory = NULL;
    return LBA_root_directory;
}

/****************************************************
* @parameters 
*   @type const char* : the file name to add as dir
*		@type mode_t: a 3digit oct number represent the 
*									access mode ie. 0777;
* @return
*   @type int: 0 means succes, 1 means fail
* This function create a new direcotry 
****************************************************/
int fs_mkdir(const char *pathname, mode_t mode){
    int success_status = 1;
    char *cwd = fs_getcwd(NULL,(DIR_MAXLENGTH + 1));
    char *fullpath = malloc(sizeof(char) * (DIR_MAXLENGTH + 1));
    // construct the full path of the given dir name
    strcpy(fullpath, cwd);
    fullpath = strcat(fullpath,"/");
    fullpath = strcat(fullpath, pathname);
    //reload directory for LBA and get the pos of next
    //direcotry entry
    fs_directory* directory = malloc(MINBLOCKSIZE);
    LBAread(directory, 1, fs_DIR.LBA_root_directory);
    reload_directory(directory);
    uint32_t free_dir_ent = find_free_dir_ent(directory);
    splitDIR *spdir = split_dir(fullpath);
    char *new_dir_name = malloc(sizeof(char)*256);
    strcpy(new_dir_name, *(spdir->dir_names + spdir->length - 1));
    spdir->length--; // move up 1 level to cwd
    //find the parent pos and check whether have duplicated name 
    uint32_t parent_pos = find_DE_pos(spdir);
    int duplicated_name = is_duplicated_dir(parent_pos, new_dir_name);
    if(parent_pos != UINT32_MAX){
        if(duplicated_name){
            printf("mkdir: %s: File exists\n", new_dir_name);
        }else{
            //initial the new direcoty entries info
            fs_de *de = (directory->d_dir_ents + free_dir_ent);
            de->de_dotdot_inode = parent_pos;
            fs_inode *inode = (directory->d_inodes + free_dir_ent);
            inode->fs_entry_type = DT_DIR;
            strcpy(de->de_name, new_dir_name);
            write_directory(directory);
            de = NULL;
            inode = NULL;
            success_status = 0;
        }
    }else{
        printf("mkdir: %s: No such file or directory\n", pathname);
    }
    free(fullpath);
    free_split_dir(spdir);
    free_directory(directory);
    free(new_dir_name);
    free(cwd);
    cwd = NULL;
    fullpath = NULL;
    new_dir_name = NULL;
    directory = NULL;
    spdir = NULL;
    return success_status;
}

/****************************************************
* @parameters 
*   @type const char* : the dir name to be removed
* This function remove a directory (if exist)
* @return
*   @type int: 0 means success, 1 means fail
* !Need to validte whether is direcotry before execute
****************************************************/
int fs_rmdir(const char *pathname){
    int success_rmdir = 0;
    char *cwd = fs_getcwd(NULL,(DIR_MAXLENGTH + 1));
    char *dirname = malloc(sizeof(char) * (DIR_MAXLENGTH + 1));
    char *fullpath = malloc(sizeof(char) * (DIR_MAXLENGTH + 1));
    char *filename = malloc(sizeof(char) * (DIR_MAXLENGTH + 1));
    //construct the fullpath of the give dir name
    strcpy(filename, pathname);
    strcpy(fullpath, cwd);
    fullpath = strcat(fullpath, "/");
    fullpath = strcat(fullpath, filename);
    // reload directory for LBA
    fs_directory* directory = malloc(MINBLOCKSIZE);
    LBAread(directory, 1, fs_DIR.LBA_root_directory);
    reload_directory(directory);
    //check whether the give name is the Dir type
    int is_dir = fs_isDir(filename);
    splitDIR *spdir = split_dir(fullpath);
    if(is_dir == 1){
        // find the directory entry position and check wheher it is empty
        uint32_t de_pos = find_DE_pos(spdir);
        fs_de *de = (directory->d_dir_ents + de_pos);
        uint32_t inode_pos = de->de_inode;
        int is_empty_dir = 1;
        for(int i = 0; i < directory->d_num_DEs; ++i){
            if((directory->d_dir_ents + i)->de_dotdot_inode == inode_pos){
                is_empty_dir = 0;
                break;
            }

        }
        if(is_empty_dir){
            // de-connected it to the parent node (which is free from directory)
            de->de_dotdot_inode = UINT32_MAX;
            success_rmdir = 1;
            //write back changed directory info to LBA
            write_directory(directory);
        }else{
            printf("%s is not a empty directory\n", de->de_name);
        }
        de = NULL;      
    }else{
        printf("rmdir: %s: No such file or directory\n", *(spdir->dir_names + spdir->length - 1));
    }
    free(fullpath);
    free_split_dir(spdir);
    free_directory(directory);
    free(cwd);
    cwd = NULL;
    dirname = NULL;
    fullpath = NULL;
    directory = NULL;
    spdir = NULL;
   return success_rmdir;
}

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
fdDir * fs_opendir(const char *name){
    splitDIR *spdir = split_dir(name);
    fdDir *dirp = malloc(sizeof(fdDir));
    dirp->cur_pos = 0;
    uint32_t de_pos = find_DE_pos(spdir);
    if(de_pos == UINT32_MAX){
        char *filename = *(spdir->dir_names + (spdir->length - 1));
        printf("no such file or direcotry: %s\n", filename);
        free_split_dir(spdir);
        free(dirp);
        spdir = NULL;
        filename = NULL;
        return NULL;
    }
    dirp->de_pos = de_pos;
    fs_directory* directory = malloc(MINBLOCKSIZE);
    LBAread(directory, 1, fs_DIR.LBA_root_directory);
    reload_directory(directory);
    uint32_t parent_inode = dirp->de_pos;
    uint32_t count_children = 0;
    uint32_t current_parent_inode;
    for(int i = 1; i < directory->d_num_DEs; ++i){
        current_parent_inode = (directory->d_dir_ents + i)->de_dotdot_inode;
        if(current_parent_inode == parent_inode){
            count_children++;
        }
    }
    dirp->num_children = count_children;
    dirp->childrens = (fs_de**)malloc(sizeof(fs_de*) * count_children);

    int pos = 0;
    for(int i = 1; i < directory->d_num_DEs; ++i){
        fs_de *current_dir_ent = (directory->d_dir_ents + i);
        current_parent_inode = current_dir_ent->de_dotdot_inode;
        if(current_parent_inode == parent_inode){
            *(dirp->childrens + pos) = current_dir_ent;
            pos++;
        }
        current_dir_ent = NULL;
        if(pos == count_children){
            break;
        }
    }
   free_directory(directory);
   directory = NULL;
   free_split_dir(spdir);
   spdir = NULL;
   return dirp;
}

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
struct fs_diriteminfo *fs_readdir(fdDir *dirp){
    fs_directory* directory = malloc(MINBLOCKSIZE);
    LBAread(directory, 1, fs_DIR.LBA_root_directory);
    reload_directory(directory);
    // Iterated the the children one by one, return NULL if finish
    if((dirp->cur_pos) < (dirp->num_children)){
        struct fs_diriteminfo* dirinfo = malloc(sizeof(struct fs_diriteminfo));
        uint32_t inode = (directory->d_dir_ents + dirp->cur_pos)->de_inode;
        unsigned char file_type = (directory->d_inodes + inode)->fs_entry_type;
        dirinfo->file_type = file_type;
        strcpy(dirinfo->d_name, (*(dirp->childrens + dirp->cur_pos))->de_name);
        (dirp->cur_pos)++;
        free_directory(directory);
        directory = NULL;
        return dirinfo;
    }else{
        free_directory(directory);
        directory = NULL;
        return NULL;
    }
}

/****************************************************
* @parameters 
*   @type fdDir* : the direcotry struct fdDir to close
* @return
*   @type int: 0 is success, 1 is fail
* This function is used to close and clean the fdDir
* object and do something to close the itreation
****************************************************/
int fs_closedir(fdDir *dirp){
    free(dirp->childrens);
    free(dirp);
    dirp->childrens = NULL;
    dirp = NULL;
    return 0;
}

/****************************************************
* @parameters 
*   @type char*: buf to store the cwd info
@		@type size_t: the size of given buf;
* @return
*   @type char*: the cwd info (not the buf);
* This function is used to get the current working
*	directory.
****************************************************/
char * fs_getcwd(char *buf, size_t size){
   char *cwd = malloc(sizeof(char)*(size + 1));
   strcpy(cwd, fs_DIR.cwd);
   if(buf != NULL){
    strcpy(buf, fs_DIR.cwd);
   }
   return cwd;
}

/****************************************************
* @parameters 
*   @type char*: the path info to change cwd
* @return
*   @type int : 0 is success, 1 is fail;
* This function is used to change the current working
* directory
****************************************************/
int fs_setcwd(char *buf){
    //To-do finshi relative path
    int is_success = 1;
    char *cwd = fs_getcwd(NULL, (DIR_MAXLENGTH + 1));
    splitDIR *spcwd = split_dir(cwd);
    splitDIR *spbuf = split_dir(buf);
    splitDIR *spdir = NULL;
    char *new_path = NULL;
    char *temp_cwd = NULL;
    char *new_parent_dir = NULL;
    char *new_dir_path = malloc(sizeof(char) * (DIR_MAXLENGTH + 1));
    //analysis the path info of cd command 
    if(strcmp(buf, ".") == 0){
        strcpy(new_dir_path, cwd);
    }else if(strcmp(buf, "..") == 0){
        if((spcwd->length) <= 2){
            strcpy(new_dir_path, "root/");
        }else{
            new_path = assemble_path(cwd, 0, -1);
            strcpy(new_dir_path, new_path);
        }
    }else if((strlen(buf) > 1) && (buf[0] == '.')){
        new_path = assemble_path(buf + 1, 0, 0);
        strcpy(new_dir_path, cwd);
        if(spcwd->length > 1)
            new_dir_path = strcat(new_dir_path, "/");
        new_dir_path = strcat(new_dir_path, new_path);
    }else if(strcmp(buf, "/") == 0){
        strcpy(new_dir_path, "root/");
    }else if((strlen(buf) > 1) && (buf[0] == '/')){
        new_path = assemble_path(buf + 1, 0, 0);
        strcpy(new_dir_path, "root/");
        new_dir_path = strcat(new_dir_path, new_path);
    }else{
        new_path = assemble_path(buf, 0, 0);
        strcpy(new_dir_path, cwd);
        if(spcwd->length > 1)
            new_dir_path = strcat(new_dir_path, "/");
        new_dir_path = strcat(new_dir_path, new_path);
    }
    /*check the url is exist as dir*/
    temp_cwd = fs_getcwd(NULL, (DIR_MAXLENGTH + 1));
    new_parent_dir = assemble_path(new_dir_path, 0, -1);
    spdir = split_dir(new_dir_path);
    strcpy(fs_DIR.cwd, new_parent_dir);
    if(fs_isDir(*(spdir->dir_names + spdir->length - 1))){
        strcpy(fs_DIR.cwd, new_dir_path);
        is_success = 0;
    }else{
        strcpy(fs_DIR.cwd, temp_cwd);
    }
    free_split_dir(spcwd);
    free_split_dir(spbuf);
    free_split_dir(spdir);
    free(new_path);
    free(temp_cwd);
    free(new_parent_dir);
    free(new_dir_path);
    free(cwd); 
    cwd = NULL;
    spcwd = NULL;
    spbuf = NULL;
    spdir = NULL;
    new_path = NULL;
    temp_cwd = NULL;
    new_parent_dir = NULL;
    new_dir_path = NULL;
    return is_success;
}

/****************************************************
* @parameters 
*   @type char*: the name of file to check whether it
*								is the File type
* @return
*   @type int: 1 is for File, 0 otherwise
* This function is used to check whether a given file
* name is the type File or not
****************************************************/
int fs_isFile(char * path){
    char *cwd = fs_getcwd(NULL,(DIR_MAXLENGTH + 1));
    char *fullpath = malloc(sizeof(char) * (DIR_MAXLENGTH + 1));
    strcpy(fullpath, cwd);
    fullpath = strcat(fullpath,"/");
    fullpath = strcat(fullpath, path);
    int is_file = is_File(fullpath);
    // fs_directory* directory = malloc(MINBLOCKSIZE);
	// LBAread(directory, 1, fs_DIR.LBA_root_directory);
	// reload_directory(directory);
    // splitDIR *spdir = split_dir(fullpath);
    // uint32_t de_pos = find_DE_pos(spdir);
    // // If the path is valid(exist)
    // if(de_pos != UINT32_MAX){
    //     uint32_t inode_num = (directory->d_dir_ents + de_pos)->de_inode;
    //     unsigned char file_type = (directory->d_inodes + inode_num)->fs_entry_type;
    //     if(file_type == DT_REG){
    //         is_file = 1;
    //     }
    // }
    free(fullpath);
    // free_split_dir(spdir);
    // free_directory(directory);
    free(cwd);
    cwd = NULL;
    fullpath = NULL;
    // directory = NULL;
    // spdir = NULL;
    return is_file;
    }//return 1 if file, 0 otherwise

/****************************************************
* @parameters 
*   @type char*: the name of file to check whether it
*								is the Dir type
* @return
*   @type int: 1 is for Dir, 0 otherwise
* This function is used to check whether a given file
* name is the type Dir or not
****************************************************/
int fs_isDir(char * path){
    char *cwd = fs_getcwd(NULL,(DIR_MAXLENGTH + 1));
    char *fullpath = malloc(sizeof(char) * (DIR_MAXLENGTH + 1));
    strcpy(fullpath, cwd);
    fullpath = strcat(fullpath, "/");
    fullpath = strcat(fullpath, path);

    int is_dir = is_Dir(fullpath);
    // fs_directory* directory = malloc(MINBLOCKSIZE);
	// LBAread(directory, 1, fs_DIR.LBA_root_directory);
	// reload_directory(directory);
    // splitDIR *spdir = split_dir(fullpath);
    // uint32_t de_pos = find_DE_pos(spdir);
    // // if the path is valid(exist), check file info
    // if(de_pos != UINT32_MAX){
    //     uint32_t inode_num = (directory->d_dir_ents + de_pos)->de_inode;
    //     unsigned char file_type = (directory->d_inodes + inode_num)->fs_entry_type;
    //     if(file_type == DT_DIR){
    //         is_dir = 1;
    //     }
    // }
    free(fullpath);
    // free_split_dir(spdir);
    // free_directory(directory);
    free(cwd);
    cwd = NULL;
    fullpath = NULL;
    // directory = NULL;
    // spdir = NULL;
    return is_dir;
}		//return 1 if directory, 0 otherwise

/****************************************************
* @parameters 
*   @type char*: the name of file to delete
* @return
*   @type int: 0 is success, 1 is fail
* This function is used to delete a File
* !Need to verify whether a given filename is File
* before calling this function to delete
****************************************************/
int fs_delete(char* filename){
    int success_delete = 1;
    fs_directory* directory = malloc(MINBLOCKSIZE);
	LBAread(directory, 1, fs_DIR.LBA_root_directory);
	reload_directory(directory);
    char *cwd = fs_getcwd(NULL,(DIR_MAXLENGTH + 1));
    char *fullpath = malloc(sizeof(char) * (DIR_MAXLENGTH + 1));
    strcpy(fullpath, cwd);
    if(strcmp(fullpath, "root/") != 0)
    fullpath = strcat(fullpath, "/");
    fullpath = strcat(fullpath, filename);
    int is_File = fs_isFile(fullpath);
    splitDIR *spdir = split_dir(fullpath);
    if(is_File == 1){
        uint32_t de_pos = find_DE_pos(spdir);
        fs_de *de = (directory->d_dir_ents + de_pos);
        uint32_t inode_pos = de->de_inode;
        de->de_dotdot_inode = UINT32_MAX;
        /*to-do free space of the file (inode find address*/
        write_directory(directory);
        success_delete = 0;
    }
    free_directory(directory);
    free(cwd);
    free(fullpath);
    free_split_dir(spdir);
    return success_delete;
}	//removes a file

/****************************************************
* @parameters 
*   @type char*: the name of file to get info for
*		@type struct fs_stat*: fs_stat struct to hold
8													file info.
* @return
*   @type int: 0 is success, 1 is fail
* This function is used for the ls command to 
* iterate the info of each file(children)
****************************************************/
int fs_stat(const char *path, struct fs_stat *buf){
    fs_directory* directory = malloc(MINBLOCKSIZE);
    LBAread(directory, 1, fs_DIR.LBA_root_directory);
    reload_directory(directory);
    char *fullpath= malloc(sizeof(char)*(DIR_MAXLENGTH + 1));
    char *cwd = fs_getcwd(NULL, (DIR_MAXLENGTH + 1));
    strcpy(fullpath, cwd);
    fullpath = strcat(fullpath, "/");
    fullpath = strcat(fullpath,path);
    splitDIR *spdir = split_dir(fullpath);
    uint32_t de_pos = find_DE_pos(spdir);
    uint32_t inode_pos =(directory->d_dir_ents + de_pos)->de_inode;
    fs_inode *inode = (directory->d_inodes + inode_pos);
    buf->st_size = inode->fs_size;
    buf->st_blksize = inode->fs_blocks;
    buf->st_blocks = inode->fs_blocks;
    buf->st_accesstime = inode->fs_accesstime;
    buf->st_modtime = inode->fs_modtime;
    buf->st_createtime = inode->fs_createtime;
    buf->st_accessmode = inode->fs_accessmode;
    free_directory(directory);
    free(fullpath);
    free_split_dir(spdir);
    free(cwd);
    fullpath = NULL;
    spdir = NULL;
    cwd = NULL;
    directory = NULL;
    inode = NULL;
    return 0;
}
