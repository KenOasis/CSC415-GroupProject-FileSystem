#include <string.h>
#include "fsLow.h"
#include "dir.h"

/****************************************************
* @parameters 
*   @type fs_directory*: directory 
* @return
*   @type uint_32: a number represent the position
*                  of the directory entry
* This function return the position of the next free
* directory entry.
****************************************************/
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

/****************************************************
* @parameters 
*   @type fs_directory*: directory
* @return
*   @type int: 0 is success, 1 is fail
* This function reload the directory from LBA space
****************************************************/
int reload_directory(fs_directory * directory){
    directory->d_inodes = malloc(MINBLOCKSIZE * (directory->d_inode_blocks));
    directory->d_dir_ents = malloc(MINBLOCKSIZE * (directory->d_de_blocks));
    LBAread(directory->d_inodes, directory->d_inode_blocks, directory->d_inode_start_location);
    LBAread(directory->d_dir_ents, directory->d_de_blocks, directory->d_de_start_location);
    return 0;
}

/****************************************************
* @parameters 
*   @type fs_directory*: directory
* @return
*   @void
*  Destructor of the directory to free the memory
*  of the struct type directory
****************************************************/
void free_directory(fs_directory *directory){
    free(directory->d_dir_ents);
    free(directory->d_inodes);
    free(directory);
}

/****************************************************
* @parameters 
*   @type fs_directory*: directory
* @return
*   @type int: 0 is success, 1 is fail
* This function write directory back to LBA
****************************************************/
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
int is_duplicated_dir(uint32_t parent_de_pos, char* name){// if return value = 0, means no duplicated
    int is_duplicated = 0;
    fs_directory* directory = malloc(MINBLOCKSIZE);
	LBAread(directory, 1, fs_DIR.LBA_root_directory);
	reload_directory(directory);
    for(int i = 1; i < directory->d_num_DEs; ++i){
        uint32_t current_parent = (directory->d_dir_ents + i)->de_dotdot_inode;
       
        int not_same_name = strcmp(name, (directory->d_dir_ents + i)->de_name);
        if((current_parent == parent_de_pos) && (!not_same_name)){
            is_duplicated = 1;
            break;
        }
    }
    free_directory(directory);
    directory = NULL;
    return is_duplicated;
}

/****************************************************
* @parameters 
*   @type const char*: path to be splited 
* @return
*   @type splitDIR*: a pointer point to a struct splitDIR
*                    which hold the splited path
* This function split the given path into seperate names
* of each level of directory/file
****************************************************/
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

/****************************************************
* @parameters 
*   @type splitDIR*: the splited path info
* @return
*   @type void
* This destructor to free the allocated memory of 
* strut type splitDIR
****************************************************/
void free_split_dir(splitDIR *spdir){
    for(int i = 0; i < spdir->length; ++i){
        free(*(spdir->dir_names + i));
    }
    free(spdir);
}

/****************************************************
* @parameters 
*   @type char*: the buf hold the path to assembly
    @type int*: the offset from the head of path
		@type int*: the offset from the tail of path
* @return
*   @type char*: the path of assembly result
* This function is to assembly path as fs_setcwd needed
* example:
*			intput:
			buf: /this/is/a/path/of/test
			head_offset: -1
			tail_offset: -1
			output:
		  /is/a/path/of
			it cut the 1st level from the head : /this
			, and the 1st level from the tail: /of  
****************************************************/
char *assemble_path(char *buf, int head_offset, int tail_offset){
    char *temp = malloc(sizeof(char) * (DIR_MAXLENGTH + 1));
    char *new_cwd = malloc(sizeof(char) * (DIR_MAXLENGTH + 1));
    strcpy(temp, buf);
    splitDIR *temp_spbuf = split_dir(temp);
    for(int i = head_offset; i < (temp_spbuf->length + tail_offset); ++i){
        if(i < (temp_spbuf->length - 1 + tail_offset)){
            strcpy(temp, *(temp_spbuf->dir_names + i));
            temp = strcat(temp,"/");
        }else{
            strcpy(temp,(*(temp_spbuf->dir_names + i)));
        }
            new_cwd = strcat(new_cwd, temp);
            strcpy(temp, "");
    }
    free(temp);
    temp = NULL;
    free_split_dir(temp_spbuf);
    temp_spbuf = NULL;
    return new_cwd;
}

/****************************************************
*  helper function to format time output, test only
****************************************************/
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

/****************************************************
*  helper function to format accessmode output, test only
****************************************************/
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
