#include "kernel/types.h"
#include "user.h"

void creat_child (int p[]); //���ڵݹ鴴���ӽ��̺͹ܵ�

int main(int argc,char* argv[]){

    int p[2];
    pipe(p);

    int ret = fork();
    if(ret == 0){
        /*�ӽ���*/
        creat_child(p); //�ݹ鴴���ӽ��̺͹ܵ�
        exit(0);

    }else if(ret > 0){
        /*������*/
        close(p[0]);    //�رչܵ�����
        for(int i = 2; i <= 35; i++){
            write(p[1],&i,sizeof(int));//��2��35д��ܵ�
        }
        close(p[1]);    //�ر�д��
        wait(0);        //�ȴ��ӽ��̷���
    }

    exit(0); 
}

void creat_child (int p[]){
    int p_child[2];
    int first_num,num;
    close(p[1]);    //�ر���һ���ܵ���д��

    if(read(p[0],&first_num,sizeof(int))){
        printf("prime %d\n",first_num); //����һ���ܵ���ȡ��һ���������
        pipe(p_child);                  //������һ���ܵ�
        
        int ret = fork();
        if (ret == 0){
            //�ӽ���
            creat_child(p_child);       //�ݹ�
        }else if(ret > 0){
            //������
            close(p_child[0]);          //�ر���һ���ܵ��Ķ���
            while (read(p[0],&num,sizeof(int)))//����һ���ܵ���ȡ����
            {
                if(num % first_num != 0){
                    write(p_child[1],&num,sizeof(int));//�ų�����һ�����ı�����д����һ���ܵ�
                }
            }
            close(p[0]);        //�ر���һ���ܵ��Ķ���
            close(p_child[1]);  //�ر���һ���ܵ���д��
            wait(0);            //�ȴ��ӽ��̷���
        }
        
    }
    exit(0);
}