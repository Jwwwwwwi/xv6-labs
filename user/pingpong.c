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
        /* �ӽ��� */
        close(parent_p[1]); //�رո����̹ܵ���д��
        close(child_p[0]);  //�ر��ӽ��̹ܵ��Ķ���

        read(parent_p[0],buf,4);    //�Ӹ����̹ܵ��Ķ��˶�ȡ����
        printf("%d: received %s\n",getpid(),buf);
        write(child_p[1],"pong",4); //���ӽ���Ҫд������д���ӽ��̹ܵ�

        close(parent_p[0]); // ��ȡ��ɣ��رն���
        close(child_p[1]);  // д����ϣ��ر�д��
        exit(0);
    } else { 
        /* ������ */

        close(parent_p[0]); //�رո����̹ܵ��Ķ���
        close(child_p[1]);  //�ر��ӽ��̹ܵ���д��

        write(parent_p[1],"ping",4);    //�Ѹ�����Ҫд������д�븸���̹ܵ�
        read(child_p[0],buf,4);         //���ӽ��̹ܵ��Ķ��˶�ȡ����
        printf("%d: received %s\n",getpid(),buf);
        
        close(child_p[0]); // д����ɣ��ر�д��
        close(parent_p[1]);// ��ȡ��ɣ��رն���
        exit(0);
    }

    exit(0); 
}
