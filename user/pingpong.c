#include "kernel/types.h"
#include "user.h"

int main(int argc,char* argv[]){

    int parent_p[2];
    int child_p[2];
    char buf[128];
    
    pipe(parent_p);
    pipe(child_p);

    int ret = fork();
    if (ret == 0) { 
        /* 子进程 */
        close(parent_p[1]); //关闭父进程管道的写端
        close(child_p[0]);  //关闭子进程管道的读端

        read(parent_p[0],buf,4);    //从父进程管道的读端读取数据
        printf("%d: received %s\n",getpid(),buf);
        write(child_p[1],"pong",4); //把子进程要写的数据写入子进程管道

        close(parent_p[0]); // 读取完成，关闭读端
        close(child_p[1]);  // 写入完毕，关闭写端
        exit(0);
    } else { 
        /* 父进程 */

        close(parent_p[0]); //关闭父进程管道的读端
        close(child_p[1]);  //关闭子进程管道的写端

        write(parent_p[1],"ping",4);    //把父进程要写的数据写入父进程管道
        read(child_p[0],buf,4);         //从子进程管道的读端读取数据
        printf("%d: received %s\n",getpid(),buf);
        
        close(child_p[0]); // 写入完成，关闭写端
        close(parent_p[1]);// 读取完成，关闭读端
        exit(0);
    }

    exit(0); 
}
