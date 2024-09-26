#include <stdio.h>  
#include <dirent.h>  
#include <sys/stat.h>  
#include <unistd.h>
#include <string.h>  
#include <stdlib.h>  
#include <sys/types.h> // 可能不是必需的，但符合POSIX标准  
#define N 256 
char *s[N];
ino_t get_inode(char *s){
    struct stat  _file ;
    stat(s,&_file);
    return _file.st_ino;
} 
char *get_name(int inode){ //根据inode在当前文件寻找文件名字
    DIR *dir;struct dirent *file;
    dir = opendir(".");
    char *s = (char *)malloc(2*N*sizeof(char));
    while((file = readdir(dir)) != NULL){
        sprintf(s,"./%s",file->d_name);
        ino_t nxt = get_inode(s);
        if(nxt == inode) {
            return s+2;
        }
    }
    perror("mypwd : error");
}



int my_pwd(){
    int cnt = 0 ; // 计数器，记录跳了几层
    for(;1;){
        ino_t a1 = get_inode(".");
        ino_t a2 = get_inode("..");
        if(a1 == a2) break;
        chdir("..");
        s[++cnt] = get_name(a1);
    }

    if(cnt==0) printf("/");
    for(int i=cnt;i>=1;i--){
        printf("/%s",s[i]);
    }printf("\n");
    return 1;
}


int main(int argc, char *argv[]) {  

    if(!my_pwd()) {
        perror("mypwd : error");
    }
    return 0;  
}