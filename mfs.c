
int fs_init(){
    return 0;
}

int fs_mkdir(const char *pathname, mode_t mode){
    return 0;
}
int fs_rmdir(const char *pathname){
    return 0;
}
fdDir * fs_opendir(const char *name){
    return NULL;
}
struct fs_diriteminfo *fs_readdir(fdDir *dirp){
    return NULL;
}
int fs_closedir(fdDir *dirp){
    return 0;
}

char * fs_getcwd(char *buf, size_t size){
    return 0;
}
int fs_setcwd(char *buf){
    return 0;
}   //linux chdir
int fs_isFile(char * path){
    return 0;
}	//return 1 if file, 0 otherwise
int fs_isDir(char * path){
    return 0;
}		//return 1 if directory, 0 otherwise
int fs_delete(char* filename){
    return 0;
}	//removes a file

int fs_stat(const char *path, struct fs_stat *buf){
    return 0;
}

