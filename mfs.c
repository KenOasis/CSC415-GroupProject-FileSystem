#include <string.h>
//#include "freeSpace.h"
#include "fsLow.h"
#include "mfs.h"
#include "vcb.h"
// void __attribute__ ((constructor)) premain()
// {   // THIS IS JUST FOR TEST 
//        fs_init(&fdDIR);
// }
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
        (inodes + i)->fs_size = sizeof(fs_de);
        (inodes + i)->fs_blksize = 0;
        (inodes + i)->fs_blocks = 0;
        (inodes + i)->fs_accesstime = time(NULL);
        (inodes + i)->fs_modtime = time(NULL);
        (inodes + i)->fs_createtime = time(NULL);
        (inodes + i)->fs_entry_type = DT_DIR;
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
    // (fs_directory*)(fs_DIR.Dir) = malloc(sizeof(fs_directory));
    // fs_DIR.Dir = directory;
    LBAwrite(directory, 1, LBA_root_directory);
    fs_DIR.LBA_root_directory = LBA_root_directory;
    free(inodes);
    free(dir_ents);
    free(directory);
    return LBA_root_directory;
}

int fs_mkdir(const char *pathname, mode_t mode){
    fs_directory* directory = malloc(MINBLOCKSIZE);
    LBAread(directory, 1, fs_DIR.LBA_root_directory);
    reload_directory(directory);
    uint32_t free_dir_ent = find_free_dir_ent(directory);
    splitDIR *spdir = split_dir(pathname);
    char *new_dir_name = malloc(sizeof(char)*256);
    strcpy(new_dir_name, *(spdir->dir_names + spdir->length - 1));
    spdir->length--; // move up 1 level to cwd
    uint32_t parent_pos = find_DE_pos(spdir);
    int duplicated_name = check_duplicated_dir(parent_pos, new_dir_name);
    if(parent_pos != UINT32_MAX){
        if(duplicated_name){
            printf("mkdir: %s: File exists\n", new_dir_name);
            free_split_dir(spdir);
            free_directory(directory);
            return 0;
        }else{
            fs_de *de = (directory->d_dir_ents + free_dir_ent);
            de->de_dotdot_inode = parent_pos;
            fs_inode *inode = (directory->d_inodes + free_dir_ent);// inode is not change if it is directory (but not file)
            strcpy(de->de_name, new_dir_name);
            write_direcotry(directory);
            free_split_dir(spdir);
            free_directory(directory);
            return 1;
        }
    }else{
        printf("mkdir: %s: No such file or directory\n", pathname);
        free_split_dir(spdir);
        free_directory(directory);
        return 0;
    }
}

int fs_rmdir(const char *pathname){
    int success_rmdir = 0;
    fs_directory* directory = malloc(MINBLOCKSIZE);
    LBAread(directory, 1, fs_DIR.LBA_root_directory);
    reload_directory(directory);
    char* path = malloc(sizeof(char)*256);
    strcpy(path, pathname);
    int is_dir = fs_isDir(path);
    splitDIR *spdir = split_dir(path);
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
    }else{
        printf("rmdir: %s: No such file or directory\n", *(spdir->dir_names + spdir->length - 1));
    }
    free(path);
    free_split_dir(spdir);
    free_directory(directory);
   return success_rmdir;
}

fdDir * fs_opendir(const char *name){
   splitDIR *spdir = split_dir(name);
   fdDir *dirp;
   dirp = malloc(sizeof(fdDir));
   dirp->cur_pos = 0;
   uint32_t de_pos = find_DE_pos(spdir);
   if(de_pos == UINT32_MAX){
       char *filename = *(spdir->dir_names + (spdir->length - 1));
       printf("no such file or direcotry: %s\n", filename);
       free_split_dir(spdir);
       return NULL;
   }
   dirp->de_pos = de_pos;
   find_childrens(dirp);
   free_split_dir(spdir);
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
        free_directory(directory);
        (dirp->cur_pos)++;
        return dirinfo;
    }else{
        free_directory(directory);
        return NULL;
    }
}
int fs_closedir(fdDir *dirp){
    for(int i = 0; i < dirp->num_children; ++i){
        free(*(dirp->childrens + i));
    }
    free(dirp->childrens);
    free(dirp);
    return 1;
}

char * fs_getcwd(char *buf, size_t size){
   char *cwd = NULL;
   if(buf != NULL){
    char *cwd = malloc(sizeof(char)*(size + 1));
    strcpy(cwd, fs_DIR.cwd);
    strcpy(buf, fs_DIR.cwd);
   }
   return cwd;
}

int fs_setcwd(char *buf){
    strcpy(fs_DIR.cwd, buf);
   return 1;
}

int fs_isFile(char * path){
    int is_file = 0;
    fs_directory* directory = malloc(MINBLOCKSIZE);
	LBAread(directory, 1, fs_DIR.LBA_root_directory);
	reload_directory(directory);
    splitDIR *spdir = split_dir(path);
    uint32_t de_pos = find_DE_pos(spdir);
    if(de_pos != UINT32_MAX){
        uint32_t inode_num = (directory->d_dir_ents + de_pos)->de_inode;
        unsigned char file_type = (directory->d_inodes + inode_num)->fs_entry_type;
        if(file_type == DT_REG){
            is_file = 1;
        }
    }
    free_split_dir(spdir);
    free_directory(directory);
    return is_file;
    }//return 1 if file, 0 otherwise

int fs_isDir(char * path){
    int is_dir = 0;
    fs_directory* directory = malloc(MINBLOCKSIZE);
	LBAread(directory, 1, fs_DIR.LBA_root_directory);
	reload_directory(directory);
    splitDIR *spdir = split_dir(path);
    uint32_t de_pos = find_DE_pos(spdir);
    if(de_pos != UINT32_MAX){
        uint32_t inode_num = (directory->d_dir_ents + de_pos)->de_inode;
        unsigned char file_type = (directory->d_inodes + inode_num)->fs_entry_type;
        if(file_type == DT_DIR){
            is_dir = 1;
        }
    }
    free_split_dir(spdir);
    free_directory(directory);
    return is_dir;
}		//return 1 if directory, 0 otherwise

int fs_delete(char* filename){
    return 0;
}	//removes a file

int fs_stat(const char *path, struct fs_stat *buf){
    fs_directory* directory = malloc(MINBLOCKSIZE);
    LBAread(directory, 1, fs_DIR.LBA_root_directory);
    reload_directory(directory);
    char *cwd = malloc(sizeof(char)*4097);
    fs_getcwd(cwd, 4097);
    char *full_path = malloc(sizeof(char)*4097);
    full_path = strcat(cwd,path);
    splitDIR *spdir = split_dir(full_path);
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

    return 0;
}

//Helper
uint32_t find_free_dir_ent(fs_directory* directory){
    uint32_t free_dir_ent = 0;
    for(int i = 0; i < directory->d_num_DEs; ++i){
        if((directory->d_dir_ents + i)->de_dotdot_inode == UINT32_MAX){
            free_dir_ent = i;
            break;
        }
    }
    if(free_dir_ent == 0){
        /**To-Do**
         * extend more directory entries and more inodes, then get the new available space
         */
        return free_dir_ent;
    }
    return free_dir_ent;
}
int reload_directory(fs_directory * directory){
    //reload Dir_info from Disk(LBA)
    directory->d_inodes = malloc(MINBLOCKSIZE * (directory->d_inode_blocks));
    directory->d_dir_ents = malloc(MINBLOCKSIZE * (directory->d_de_blocks));
    LBAread(directory->d_inodes, directory->d_inode_blocks, directory->d_inode_start_location);
    LBAread(directory->d_dir_ents, directory->d_de_blocks, directory->d_de_start_location);
    //End of reload Dir_info
    return 0;
}
// @return value: UINT32_MAX means failed to find pos.
void free_directory(fs_directory *directory){
    free(directory->d_dir_ents);
    free(directory->d_inodes);
    free(directory);
}
int write_direcotry(fs_directory *directory){
    fs_de * dir_ents = directory->d_dir_ents;
    fs_inode *inodes = directory->d_inodes;
    LBAwrite(dir_ents, directory->d_de_blocks, directory->d_de_start_location);
    LBAwrite(inodes, directory->d_inode_blocks, directory->d_inode_start_location);
    LBAwrite(directory, 1, fs_DIR.LBA_root_directory);
    return 1;
}
uint32_t find_DE_pos(splitDIR *spdir){
    fs_directory* directory = malloc(MINBLOCKSIZE);
	LBAread(directory, 1, fs_DIR.LBA_root_directory);
	reload_directory(directory);
    int found_dir_count = 0;
    int parent_pos = 0;
    int current_parent_pos = 0;
    uint32_t pos = 0;
    for(int i = 0; i < (spdir->length); ++i){
        for(int j = 0; j < (directory->d_num_DEs); ++j){
            int not_equal_names = strcmp(*(spdir->dir_names + i),(directory->d_dir_ents + j)->de_name);
            current_parent_pos = (directory->d_dir_ents + j)->de_dotdot_inode;
            int is_child = (current_parent_pos == parent_pos);
            // printf("current parrent position: %d\n", current_parent_pos);
            // printf("parent position is %d\n", parent_pos);
            // printf("is chlid: %d\n", is_child);
            // printf("name equal: %d\n", !not_equal_names);
            if((!not_equal_names) && is_child){
                found_dir_count++;
                pos = (directory->d_dir_ents + j) ->de_inode;
                parent_pos = pos;
                break;
            }
        }
        if(found_dir_count < i){
                break;
        }
    }
    if(found_dir_count == spdir->length){
        free_directory(directory);
        return pos;
    }else{
        free_directory(directory);
        return UINT32_MAX;
    }
}

int check_duplicated_dir(uint32_t parent_de_pos, char* name){// if return value = 0, means no duplicated
    fs_directory* directory = malloc(MINBLOCKSIZE);
	LBAread(directory, 1, fs_DIR.LBA_root_directory);
	reload_directory(directory);
    for(int i = 1; i < directory->d_num_DEs; ++i){
        uint32_t current_parent = (directory->d_dir_ents + i)->de_dotdot_inode;
       
        int not_same_name = strcmp(name, (directory->d_dir_ents + i)->de_name);
        if((current_parent == parent_de_pos) && (!not_same_name)){
            free_directory(directory);
            return 1;
        }
    }
    free_directory(directory);
    return 0;
}

int find_childrens(fdDir *dirp){
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
    for(int i = 0; i < count_children; ++i){
        *(dirp->childrens + i) = (fs_de*)malloc(sizeof(fs_de));
    }
    int pos = 0;
    for(int i = 1; i < directory->d_num_DEs; ++i){
        fs_de *current_dir_ent = (directory->d_dir_ents + i);
        current_parent_inode = current_dir_ent->de_dotdot_inode;
        if(current_parent_inode == parent_inode){
            *(dirp->childrens + pos) = current_dir_ent;
            pos++;
        }
        if(pos == count_children){
            break;
        }
    }
    free_directory(directory);
    return 1;
}
splitDIR* split_dir(const char *name){
    // restart here! add . and .. into childrens
    const char delimiter[2] = "/";
    char *pathname = malloc(sizeof(char)*256);
    char *token;
    int length = 0;
    strcpy(pathname, name);
    token = strtok(pathname, delimiter);
    while(token != NULL){
        length++;
        token = strtok(NULL, delimiter);
    }
    splitDIR *sDir = malloc(sizeof(splitDIR));
    sDir->dir_names = (char**)malloc(sizeof(char *) * length);
    sDir->length = length;
    for(int i = 0; i < length; ++i){
        *(sDir->dir_names + i) = malloc(sizeof(char) * 256);
    }
    // remember to free outside the function
    strcpy(pathname, name);
    token = strtok(pathname, delimiter);
    int i = 0;
    while ((token != NULL) && (strcmp(token, "") != 0)) 
    {
        strcpy(*(sDir->dir_names + i), token);
        i++;
        token = strtok(NULL, delimiter);
    }
    free(pathname);
    return sDir;    
}

void free_split_dir(splitDIR *spdir){
    for(int i = 0; i < spdir->length; ++i){
        free(*(spdir->dir_names + i));
    }
    free(spdir);
}
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