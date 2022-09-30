#include "kernel/types.h"
#include "user/user.h"
#include "kernel/param.h"

int main(int argc, char *argv[])
{
  char buf[128];                  //缓冲区
  char words[128];                //后续的输入内容
  char *exe_argv[MAXARG];
  int i;
  
  for(i = 1; i < argc; i++){
    exe_argv[i-1] = argv[i];      //把xargs去掉，得到exec需要执行的参数
  }

  int argv_size = argc-1;
  int k;

  while ((k = read(0,buf,sizeof(buf))) != 0 )
  {
    char *word = words;
    for(int j = 0; j < k; j++){ 
      if(buf[j] == ' ')
      {
        words[j] = 0;                 //遇到空格说明读到了一个单词，以0结尾
        exe_argv[argv_size++] = word; //补充到需要执行的参数列表
        word = words + j + 1;         //指针移到下一个单词的位置
      }
      else if(buf[j] == '\n'){
        words[j] = 0;                 //遇到回车说明一行读完了，最后一个单词也需以0结尾
        exe_argv[argv_size++] = word; //补充参数
        exe_argv[argv_size] = 0;      //参数列表以0结尾
        word = words;                 //重置指针
        argv_size = argc - 1;         //重置参数列表

       if(fork() == 0){
          exec(exe_argv[0],exe_argv); //子进程去执行该行命令
        }else 
        {
          wait(0);                    //父进程等待子进程结束
        }
      }
      else{
        words[j] = buf[j];
      }
    }
  }
    exit(0);
}