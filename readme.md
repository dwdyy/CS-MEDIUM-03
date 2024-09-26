## 头文件

- `#include <dirent.h>`

#### dirent

该头文件提供了一些检索和遍历文件的接口，在了解该库之前，我们要先了解一下 `dirent`

```
struct dirent
{
   long d_ino; /* inode number 索引节点号 */
   off_t d_off; /* offset to this dirent 在目录文件中的偏移 */
   unsigned short d_reclen; /* length of this d_name 文件名长 */
   unsigned char d_type; /* the type of d_name 文件类型 */
   char d_name [NAME_MAX+1]; /* file name (null-terminated) 文件名，最长255字符 */
}
```

我们一个一个了解相关变量

首先在 $linux$ 中，每个文件都有一个 $inode$ ，中文叫索引节点，包含该文件的信息

我们可以使用 `stat Bignum.c ` 查看 索引节点

```

  Size: 4715            Blocks: 16         IO Block: 4096   regular file
Device: 820h/2080d      Inode: 338636      Links: 1
Access: (0644/-rw-r--r--)  Uid: ( 1000/   dwdyy)   Gid: ( 1000/   dwdyy)
Access: 2024-09-23 22:20:51.697682048 +0800
Modify: 2024-09-23 18:40:53.983631834 +0800
Change: 2024-09-23 22:20:51.657682059 +0800
 Birth: 2024-09-22 14:56:02.080299115 +0800
```

- `Size` 文件大小，单位字节

- `Blocks` 磁盘块数

- `IO Block` 每个磁盘块的字节数

- ` regular file` 文件类型

  在linux中有多种文件类型

  ```
  普通文件 -> 一般文件都是 
  目录 -> 可以类比文件夹,但linux文件夹也是文件
  符号链接 -> 快捷方式(
  块设备文件 -> 以块为单位数据的设备 硬盘 u盘
  字符设备文件 -> 以字符为单位鼠标键盘
  管道文件 -> 用于进程通信
  套接字 这个暂时没看明白
  ```

- `Device` 文件所处的设备编号

- ` Inode` 文件 inode编号

- `Links` 链接数 多个文件名可以指向同一个inode，links就是链接数，当为0的时候就会回收inode与空间

  此外又分为硬链接与软连接

  硬链接可以理解为一个文件有好多入口，都是一个inode，算入链接数

  软连接就是快捷方式 不同文件，不同inode，不算入链接数

- `Access` 文件访问权限，前面是八进制，后面是符号表示

  分别是 所有者 同组用户 其他用户

   rwx分别读写执行
  
- `uid` ~~是二字游戏~~ 是 文件所有者的id和用户名

- `Gid` 是文件所属用户组的GID和组名 

- `Access/Modify/Change/Birth` 文件访问/文件修改/文件属性(除文件内容的其他属性)/生成日期

- 数据块指针，也就是数据块的位置，但stat命令并不会返回数据块指针

参考资料

[理解inode - 阮一峰的网络日志 (ruanyifeng.com)](https://ruanyifeng.com/blog/2011/12/inode.html)



回到dirent类型我们继续来看

`d_off` 表示文件的偏移量

剩下的就很好理解了

#### dirstream  

上面是一个文件的表示，我们再来研究一下目录流的表示

关于我的理解放在注释里了

```
struct __dirstream   
   {   
    void *__fd;    //文件描述符,当打开一个文件时，内核返回一个索引，标识这个文件
    char *__data;    // 数据缓冲区，保存了这个目录读取的文件和子目录的信息，但不会全部保存，随读随存
    int __entry_data;    // 手动计数用的，一般表示处理的条目数量
    char *__ptr;    //当前目录条目在数据缓冲区中的位置
    int __entry_ptr;    // 手动计数用的，一般表示当前目录在目录流中的位置
    size_t __allocation;    // 缓冲区大小
    size_t __size;    // 缓冲区有效数据的大小（用了多少
    __libc_lock_define (, __lock)    // 多线程相关的，还没看懂
   };   
  
typedef struct __dirstream DIR;  
```

#### 相关函数

```
DIR *opendir(const char *dirname); //打开一个文件夹
struct dirent *readdir(DIR *dirp); // 读取目录
int closedir(DIR *dirp); // 关闭目录
long telldir(DIR *dp);//返回当前目录流的位值
void seekdir(DIR *dp,long loc);  // 根据位置回到目录流的指针
dirfd
```

所以我们现在可以写一个很简陋的 `ls` 了

```
#include <stdio.h>  
#include <dirent.h>  
int main() {  
    DIR * dir;
    struct dirent * dnt;
    dir = opendir("./tmp");
    if(dir!=NULL){
        while((dnt = readdir(dir)) !=NULL){
            printf("%s\n",dnt->d_name);
        }
        closedir(dir);
    }else {
        printf("Could't find it");
    }
}
```

可以看到返回了隐藏目录和两个文件

```
.
..
bbb.c
aaa.c
```

由上文我们在 dirent 类型中有 索引节点，本来我想通过索引节点直接获取文件信息

但查阅资料发现文件系统并不提供一种直接根据inode号查询文件信息的接口

我们需要一种别的方法

#### sys/stat.h

struct stat 储存文件信息(这个就是我上面提到的inode内容)

```
// 网上复值的，格式有点怪，但不影响理解，与上文的inode内容基本一致
struct stat {
        mode_t     st_mode;       //文件对应的模式，文件，目录等

        ino_t      st_ino;       //inode节点号

        dev_t      st_dev;        //设备号码

        dev_t      st_rdev;       //特殊设备号码

        nlink_t    st_nlink;      //文件的连接数

        uid_t      st_uid;        //文件所有者

        gid_t      st_gid;        //文件所有者对应的组

        off_t      st_size;       //普通文件，对应的文件字节数

        time_t     st_atime;      //文件最后被访问的时间

        time_t     st_mtime;      //文件内容最后被修改的时间

        time_t     st_ctime;      //文件状态改变时间

        blksize_t st_blksize;    //文件内容对应的块大小

        blkcnt_t   st_blocks;     //伟建内容对应的块数量
};
```

有两种方法获取状态信息

``` 
int stat(const char *path, struct stat *buf); // 根据文件路径
int fstat(int fd, struct stat *buf);//根据文件描述符
lstat // 和stat一样，但如果是个符号链接，不会去找其指向文件，直接返回链接本身信息
```

这里我采用构建路径的方法通过stat获得状态

## 识别代码

> 上述程序的头文件你见过哪些，没见过哪些？

有的上面已经写了

上面还有个 $sys/types.h$ 没有介绍，它是规范定义系统操作中各种数据类型的，mode_t,uid_t都是里面定义的

规范定义了变量，保证各个系统都可以允许，有利于下边输出时间

> 程序的运行从哪个地方开始？

从 $main$ 函数开始

> argc，*argv[]作为主函数的运行参数，它们的含义是什么？是在哪里输入的？

表示命令行运行时的参数,argc为个数,

```
./a -o 
```

此时`argc=2 argv[] = {./a,-o}`

剩下的问题我会在补全的时候写上

#### Step1

先来分析main函数，这里我们的 $ls$ 参数表示多个地址，没有时默认列出当前目录

```
int main(int argc, char *argv[]) {  
    if (argc < 2) {  
        list_directory("."); // 默认列出当前目录  
    } else {  
        for (int i = 1; i < argc; i++) {  
            list_directory(argv[i]);  // 分别列出多个目录
        }  
    }  
  
    return 0;  
}
```

#### Step2

分析一下 文件路径信息的处理

```
void list_directory(const char *path) {  
    DIR *dir;  
    struct dirent *entry;  
    struct stat statbuf;  
  
    if (!(dir = opendir(path))) {  // 没有找到对应目录，输出错误，直接停止
        perror("opendir");  
        exit(EXIT_FAILURE);  
    }  
  
    while ((entry = readdir(dir)) != NULL) {  // 跳到下一个文件，如果还有就继续
        char full_path[1024];  // 储存完整路径
        snprintf(full_path, sizeof(full_path), "%s/%s", path, entry->d_name);  
  		// 把path+文件名字放到full_path中 作为 读取文件的地址
        if (stat(full_path, &statbuf) == -1) {  //根据文件路径获取文件信息
            perror("stat");    // 没找到这个文件
            continue;  
        }  
  
        print_file_info(entry->d_name, &statbuf);  // 输出该文件信息
    }  
    closedir(dir);  
}  
```

#### Step3

这个函数输出文件相关信息

```
void print_file_info(const char *name, struct stat *statbuf) {  
    printf("%s\t", name);  //文件名
    printf("%ld\t", statbuf->st_size); // 文件大小  
    printf("%o\t", statbuf->st_mode & 0777); // 文件权限（仅显示最后三位）  
    // 最后修改时间  
    char timebuf[80];  
    strftime(timebuf, sizeof(timebuf), "%Y-%m-%d %H:%M", localtime(&statbuf->st_mtime));  
    // 格式化时间和日期 其中localtime将time_t转化为当地时间
    printf("%s\n", timebuf);  
}  
```

![](image/image1)

比较一下可以看到myls没有处理隐藏文件夹

```
if(entry->d_name[0] =='.') continue; // 可以加上这句话识别隐藏文件夹
```

## 举一反三 

想要写 `pwd`  

我的想法是不断往父文件跳，直道无法跳为止，所以我们需要函数支持改变工作目录

```
#include <unistd.h>
int chdir(const char *path);
// 改变工作目录，成功返回0
```

我们需要判断是否到根目录，一开始我的想法是，判断有无 `..` 文件，但是运行时发现根目录也有 `..` 并指向他自己，造成死循环。

查阅资料发现根目录的 `..` 指向自己所以它的 `.` 和 `..` 文件相同 inode文件就相同，我们可以基于此判断

最后一个问题就是 inode中不包含文件名字，想要获得文件名字，我们可以从父文件夹遍历，找到对应文件

首先我们根据路径获取inode

```c
ino_t get_inode(char *s){
    struct stat  _file ;
    stat(s,&_file);
    return _file.st_ino;
} 
```

我们再根据inode在当前文件寻找文件名字，并返回文件名

```c
char *get_name(int inode){ //根据inode在当前文件寻找文件名字
    DIR *dir;struct dirent *file;
    dir = opendir(".");//打开当前文件夹
    char *s = (char *)malloc(2*N*sizeof(char)); // 最长255个字符，保险我们申请两倍空间
    while((file = readdir(dir)) != NULL){
        sprintf(s,"./%s",file->d_name);// "./文件名字" 构造位置
        ino_t nxt = get_inode(s); 
        if(nxt == inode) {// 比较一下是不是一样
            return s+2;// 一样的话就省去前面两个"./" 直接返回文件名的指针
        }
    }
    perror("mypwd : error");
}
```

实现上面两个函数后下面是主函数

```c
int my_pwd(){
    int cnt = 0 ; // 计数器，记录跳了几层
    for(;1;){
        ino_t a1 = get_inode("."); 
        ino_t a2 = get_inode("..");
        if(a1 == a2) break;// 判断 . 与 .. 的inode 是否相同
        chdir("..");// 进入上一级并获取名字
        s[++cnt] = get_name(a1);
    }

	// 倒序输出，也可以递归
    for(int i=cnt;i>=1;i--){
        printf("/%s",s[i]);
    }printf("\n");
    return 1;
}
```

最后我们运行一下

```c
./mypwd
/home/dwdyy/learnc/cs6
pwd
/home/dwdyy/learnc/cs6
```

两个对比一下感觉还可以

但是在测试时发现根目录下不会输出任何东西，而pwd会输出`/`，所以我们需要特判一下

`if(cnt==0) printf("/");`

以下是完整代码

```c
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
```

相关资料

[struct stat结构体简介 - 楚云之南 - 博客园 (cnblogs.com)](https://www.cnblogs.com/CSU-PL/archive/2013/06/06/3120757.html)

([C语言实现linux pwd命令_c语言pwd.h-CSDN博客](https://blog.csdn.net/fjy19950504/article/details/69681183))

部分资料来自chatgpt
