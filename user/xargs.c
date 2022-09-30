#include "kernel/types.h"
#include "user/user.h"
#include "kernel/param.h"

int main(int argc, char *argv[])
{
  char buf[128];                  //������
  char words[128];                //��������������
  char *exe_argv[MAXARG];
  int i;
  
  for(i = 1; i < argc; i++){
    exe_argv[i-1] = argv[i];      //��xargsȥ�����õ�exec��Ҫִ�еĲ���
  }

  int argv_size = argc-1;
  int k;

  while ((k = read(0,buf,sizeof(buf))) != 0 )
  {
    char *word = words;
    for(int j = 0; j < k; j++){ 
      if(buf[j] == ' ')
      {
        words[j] = 0;                 //�����ո�˵��������һ�����ʣ���0��β
        exe_argv[argv_size++] = word; //���䵽��Ҫִ�еĲ����б�
        word = words + j + 1;         //ָ���Ƶ���һ�����ʵ�λ��
      }
      else if(buf[j] == '\n'){
        words[j] = 0;                 //�����س�˵��һ�ж����ˣ����һ������Ҳ����0��β
        exe_argv[argv_size++] = word; //�������
        exe_argv[argv_size] = 0;      //�����б���0��β
        word = words;                 //����ָ��
        argv_size = argc - 1;         //���ò����б�

       if(fork() == 0){
          exec(exe_argv[0],exe_argv); //�ӽ���ȥִ�и�������
        }else 
        {
          wait(0);                    //�����̵ȴ��ӽ��̽���
        }
      }
      else{
        words[j] = buf[j];
      }
    }
  }
    exit(0);
}