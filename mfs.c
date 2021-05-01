#include <string.h>
#include "freeSpace.h"
#include "fsLow.h"
#include "mfs.h"
#include "vcb.h"
#include "dir.h"
// int main(int argc, char * argv[]){
//        printf("LBA is %llu\n", fdDIR.directoryStartLocation);
//        return 0;
// }

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

int fs_mkdir(const char *pathname, mode_t mode){
    int success_status = 0;
    char *cwd = fs_getcwd(NULL,(DIR_MAXLENGTH + 1));
    char *fullpath = malloc(sizeof(char) * (DIR_MAXLENGTH + 1));
    cwd = strcat(cwd,"/");
    fullpath = strcat(cwd, pathname);
    fs_directory* directory = malloc(MINBLOCKSIZE);
    LBAread(directory, 1, fs_DIR.LBA_root_directory);
    reload_directory(directory);
    uint32_t free_dir_ent = find_free_dir_ent(directory);
    splitDIR *spdir = split_dir(fullpath);
    char *new_dir_name = malloc(sizeof(char)*256);
    strcpy(new_dir_name, *(spdir->dir_names + spdir->length - 1));
    spdir->length--; // move up 1 level to cwd
    uint32_t parent_pos = find_DE_pos(spdir);
    int duplicated_name = is_duplicated_dir(parent_pos, new_dir_name);
    if(parent_pos != UINT32_MAX){
        if(duplicated_name){
            printf("mkdir: %s: File exists\n", new_dir_name);
            success_status = -1;
        }else{
            fs_de *de = (directory->d_dir_ents + free_dir_ent);
            de->de_dotdot_inode = parent_pos;
            fs_inode *inode = (directory->d_inodes + free_dir_ent);
            inode->fs_entry_type = DT_DIR;
            strcpy(de->de_name, new_dir_name);
            write_direcotry(directory);
            de = NULL;
            inode = NULL;
            success_status = 0;
        }
    }else{
        printf("mkdir: %s: No such file or directory\n", pathname);
        success_status = -1;
    }
    free(fullpath);
    free_split_dir(spdir);
    free_directory(directory);
    free(new_dir_name);
    fullpath = NULL;
    new_dir_name = NULL;
    directory = NULL;
    spdir = NULL;
    return success_status;
}

int fs_rmdir(const char *pathname){
    int success_rmdir = 0;
    char *cwd = fs_getcwd(NULL,(DIR_MAXLENGTH + 1));
    char *dirname = malloc(sizeof(char) * (DIR_MAXLENGTH + 1));
    char *fullpath = malloc(sizeof(char) * (DIR_MAXLENGTH + 1));
    cwd = strcat(cwd, "/");
    fullpath = strcat(cwd, pathname);
    fs_directory* directory = malloc(MINBLOCKSIZE);
    LBAread(directory, 1, fs_DIR.LBA_root_directory);
    reload_directory(directory);
    strcpy(dirname, pathname);
    int is_dir = fs_isDir(dirname);
    splitDIR *spdir = split_dir(fullpath);
    if(is_dir == 1){
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
            de->de_dotdot_inode = UINT32_MAX;
            success_rmdir = 1;
            write_direcotry(directory);
        }else{
            printf("%s is not a empty directory\n", de->de_name);
        }
        de = NULL;      
    }else{
        printf("rmdir: %s: No such file or directory\n", *(spdir->dir_names + spdir->length - 1));
    }
    free(fullpath);
    free(dirname);
    free_split_dir(spdir);
    free_directory(directory);
    cwd = NULL;
    dirname = NULL;
    fullpath = NULL;
    directory = NULL;
    spdir = NULL;
   return success_rmdir;
}

fdDir * fs_opendir(const char *name){
   splitDIR *spdir = split_dir(name);
   fdDir *dirp = NULL;
   dirp = malloc(sizeof(fdDir));
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
   find_childrens(dirp);
   free_split_dir(spdir);
   spdir = NULL;
   return dirp;
}

struct fs_diriteminfo *fs_readdir(fdDir *dirp){
    fs_directory* directory = malloc(MINBLOCKSIZE);
    LBAread(directory, 1, fs_DIR.LBA_root_directory);
    reload_directory(directory);
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
int fs_closedir(fdDir *dirp){
    free(dirp->childrens);
    free(dirp);
    dirp->childrens = NULL;
    dirp = NULL;
    return 1;
}

char * fs_getcwd(char *buf, size_t size){
   char *cwd = malloc(sizeof(char)*(size + 1));
   strcpy(cwd, fs_DIR.cwd);
   if(buf != NULL){
    strcpy(buf, fs_DIR.cwd);
   }
   return cwd;
}

int fs_setcwd(char *buf){
    int is_success = 1;
    char *cwd = fs_getcwd(NULL, (DIR_MAXLENGTH + 1));
    splitDIR *spcwd = split_dir(cwd);
    splitDIR *spbuf = split_dir(buf);
    splitDIR *spdir = NULL;
    char *new_path = NULL;
    char *temp_cwd = NULL;
    char *new_parent_dir = NULL;
    char *new_dir_path = malloc(sizeof(char) * (DIR_MAXLENGTH + 1));
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
        if(spcwd->length > 1)
            cwd = strcat(cwd, "/");
        new_dir_path = strcat(cwd, new_path);
    }else if(strcmp(buf, "/") == 0){
        strcpy(new_dir_path, "root/");
    }else if((strlen(buf) > 1) && (buf[0] == '/')){
        new_path = assemble_path(buf + 1, 0, 0);
        strcpy(cwd, "root/");
        new_dir_path = strcat(cwd, new_path);
    }else{
        new_path = assemble_path(buf, 0, 0);
        if(spcwd->length > 1)
            cwd = strcat(cwd, "/");
        new_dir_path = strcat(cwd, new_path);
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
    // free(cwd); //free will cause error????
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

int fs_isFile(char * path){
    int is_file = 0;
    char *cwd = fs_getcwd(NULL,(DIR_MAXLENGTH + 1));
    char *fullpath = malloc(sizeof(char) * (DIR_MAXLENGTH + 1));
    cwd = strcat(cwd,"/");
    fullpath = strcat(cwd, path);
    fs_directory* directory = malloc(MINBLOCKSIZE);
	LBAread(directory, 1, fs_DIR.LBA_root_directory);
	reload_directory(directory);
    splitDIR *spdir = split_dir(fullpath);
    uint32_t de_pos = find_DE_pos(spdir);
    if(de_pos != UINT32_MAX){
        uint32_t inode_num = (directory->d_dir_ents + de_pos)->de_inode;
        unsigned char file_type = (directory->d_inodes + inode_num)->fs_entry_type;
        if(file_type == DT_REG){
            is_file = 1;
        }
    }
    free(fullpath);
    free_split_dir(spdir);
    free_directory(directory);
    cwd = NULL;
    fullpath = NULL;
    directory = NULL;
    spdir = NULL;
    return is_file;
    }//return 1 if file, 0 otherwise

int fs_isDir(char * path){
    int is_dir = 0;
    char *cwd = fs_getcwd(NULL,(DIR_MAXLENGTH + 1));
    char *fullpath = malloc(sizeof(char) * (DIR_MAXLENGTH + 1));
    cwd = strcat(cwd, "/");
    fullpath = strcat(cwd, path);
    fs_directory* directory = malloc(MINBLOCKSIZE);
	LBAread(directory, 1, fs_DIR.LBA_root_directory);
	reload_directory(directory);
    splitDIR *spdir = split_dir(fullpath);
    uint32_t de_pos = find_DE_pos(spdir);
    if(de_pos != UINT32_MAX){
        uint32_t inode_num = (directory->d_dir_ents + de_pos)->de_inode;
        unsigned char file_type = (directory->d_inodes + inode_num)->fs_entry_type;
        if(file_type == DT_DIR){
            is_dir = 1;
        }
    }
    free(fullpath);
    free_split_dir(spdir);
    free_directory(directory);
    cwd = NULL;
    fullpath = NULL;
    directory = NULL;
    spdir = NULL;
    return is_dir;
}		//return 1 if directory, 0 otherwise

int fs_delete(char* filename){
    return 0;
}	//removes a file

int fs_stat(const char *path, struct fs_stat *buf){
    fs_directory* directory = malloc(MINBLOCKSIZE);
    LBAread(directory, 1, fs_DIR.LBA_root_directory);
    reload_directory(directory);
    char *fullpath= malloc(sizeof(char)*(DIR_MAXLENGTH + 1));
    char *cwd = fs_getcwd(NULL, (DIR_MAXLENGTH + 1));
    cwd = strcat(cwd, "/");
    fullpath = strcat(cwd,path);
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
    buf->st_file_address = inode->fs_address;
    free_directory(directory);
    free(fullpath);
    free_split_dir(spdir);
    fullpath = NULL;
    spdir = NULL;
    cwd = NULL;
    directory = NULL;
    inode = NULL;
    return 0;
}

