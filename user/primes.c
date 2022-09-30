#include "kernel/types.h"
#include "user.h"

void creat_child (int p[]); //用于递归创建子进程和管道

int main(int argc,char* argv[]){

    int p[2];
    pipe(p);

    int ret = fork();
    if(ret == 0){
        /*子进程*/
        creat_child(p); //递归创建子进程和管道
        exit(0);

    }else if(ret > 0){
        /*父进程*/
        close(p[0]);    //关闭管道读端
        for(int i = 2; i <= 35; i++){
            write(p[1],&i,sizeof(int));//把2到35写入管道
        }
        close(p[1]);    //关闭写端
        wait(0);        //等待子进程返回
    }

    exit(0); 
}

void creat_child (int p[]){
    int p_child[2];
    int first_num,num;
    close(p[1]);    //关闭上一根管道的写端

    if(read(p[0],&first_num,sizeof(int))){
        printf("prime %d\n",first_num); //从上一根管道读取第一个数并输出
        pipe(p_child);                  //创建下一根管道
        
        int ret = fork();
        if (ret == 0){
            //子进程
            creat_child(p_child);       //递归
        }else if(ret > 0){
            //父进程
            close(p_child[0]);          //关闭下一根管道的读端
            while (read(p[0],&num,sizeof(int)))//从上一根管道读取数据
            {
                if(num % first_num != 0){
                    write(p_child[1],&num,sizeof(int));//排除掉第一个数的倍数，写入下一根管道
                }
            }
            close(p[0]);        //关闭上一根管道的读端
            close(p_child[1]);  //关闭下一根管道的写端
            wait(0);            //等待子进程返回
        }
        
    }
    exit(0);
}