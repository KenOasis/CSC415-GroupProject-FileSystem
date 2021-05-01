#include <string.h>
#include "fsLow.h"
#include "fs_dir.h"
#include "mfs.h"
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
    dir_ents = NULL;
    inodes = NULL;
    return 0;
}
uint32_t find_DE_pos(splitDIR *spdir){
    uint32_t de_pos = UINT32_MAX;
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
        de_pos = pos;

    }
    free(directory);
    directory = NULL;
    return de_pos;
}

int check_duplicated_dir(uint32_t parent_de_pos, char* name){// if return value = 0, means no duplicated
    int is_duplicated = 0;
    fs_directory* directory = malloc(MINBLOCKSIZE);
	LBAread(directory, 1, fs_DIR.LBA_root_directory);
	reload_directory(directory);
    for(int i = 1; i < directory->d_num_DEs; ++i){
        uint32_t current_parent = (directory->d_dir_ents + i)->de_dotdot_inode;
       
        int not_same_name = strcmp(name, (directory->d_dir_ents + i)->de_name);
        if((current_parent == parent_de_pos) && (!not_same_name)){
            free_directory(directory);
            is_duplicated = 1;
            break;
        }
    }
    free_directory(directory);
    directory = NULL;
    return is_duplicated;
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
        current_dir_ent = NULL;
        if(pos == count_children){
            break;
        }
    }
    free_directory(directory);
    directory = NULL;
    return 0;
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
    pathname = NULL;
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
struct fs_diriteminfo* getDirInfo(){
    return NULL;
}