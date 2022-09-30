#include "kernel/types.h"
#include "kernel/stat.h"
#include "kernel/fs.h"
#include "user.h"

char*
fmtname(char *path)
{
  static char buf[DIRSIZ+1];
  char *p;

  // Find first character after last slash.
  for(p=path+strlen(path); p >= path && *p != '/'; p--)
    ;
  p++;

  // Return blank-padded name.
  if(strlen(p) >= DIRSIZ)
    return p;
  memmove(buf, p, strlen(p));
  memset(buf+strlen(p), ' ', DIRSIZ-strlen(p));
  return buf;
}

void find(char *path, char* target)
{
  char buf[512], *p;
  int fd;
  struct dirent de;
  struct stat st;
  /*fmtname函数中取得的文件名除去名字外其余被空格填满，会影响到strcmp的判断，需要对target做相同的操作*/
  memset(target+strlen(target),' ',DIRSIZ-strlen(target));
                                                                                                                        
  if((fd = open(path, 0)) < 0){                                           
    fprintf(2, "find: cannot open %s\n", path);
    return;
  }

  if(fstat(fd, &st) < 0){
    fprintf(2, "find: cannot stat %s\n", path);
    close(fd);
    return;
  }

  switch(st.type){
  case T_FILE:
    if (strcmp(fmtname(path),target) == 0){ //将打开的文件名和目标文件名对比
        printf("%s\n",path);
    }
    break;

  case T_DIR:
    if(strlen(path) + 1 + DIRSIZ + 1 > sizeof buf){
      printf("find: path too long\n");
      break;
    }
    strcpy(buf, path);
    p = buf+strlen(buf);
    *p++ = '/';
    while(read(fd, &de, sizeof(de)) == sizeof(de)){
      if(de.inum == 0)
        continue;
      //避免递归到.和..
      if(strcmp(de.name,".") == 0 ||(strcmp(de.name,"..") == 0)){
        continue;
      }
      memmove(p, de.name, DIRSIZ);
      p[DIRSIZ] = 0;
      if(stat(buf, &st) < 0){
        printf("find: cannot stat %s\n", buf);
        continue;
      }
      find(buf,target); //递归到子目录
    }
    break;
  }
  close(fd);
}

int main(int argc,char* argv[]){

    if(argc < 3){
        printf("find need 2 arguments!\n"); //检查参数数量是否正确
        exit(-1);
    }
    find(argv[1],argv[2]);
    exit(0); 
}
