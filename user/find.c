#include "kernel/types.h"
#include "kernel/stat.h"
#include "kernel/fs.h"
#include "user/user.h"
#include "kernel/fcntl.h"
#include "kernel/param.h"
#include "kernel/param.h"

int match(char *re, char *text);

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
  memset(buf+strlen(p), 0, DIRSIZ-strlen(p));
  return buf;
}

void
find(char *path, char *name, int doexec, char *cmd[])
{
  char buf[512], *p;
  int fd;
  struct dirent de;
  struct stat st;

  if((fd = open(path, O_RDONLY)) < 0){
    fprintf(2, "find: cannot open %s\n", path);
    return;
  }

  if(fstat(fd, &st) < 0){
    fprintf(2, "find: cannot stat %s\n", path);
    close(fd);
    return;
  }

  if(st.type == T_FILE){
    if(match(name, fmtname(path))){
      if(doexec){
  int pid = fork();
  if(pid == 0){
    // child: build argv array
    char *argv[MAXARG];
    int i = 0;
    while(cmd[i]){
      argv[i] = cmd[i];
      i++;
    }
    argv[i] = path;
    argv[i+1] = 0;
    exec(argv[0], argv);
    exit(1); // exec failed
  } else {
    wait(0);
  }
} else {
  printf("%s\n", path);
}

    }
  } else if(st.type == T_DIR){
    if(strlen(path) + 1 + DIRSIZ + 1 > sizeof buf){
      printf("find: path too long\n");
    } else {
      strcpy(buf, path);
      p = buf+strlen(buf);
      *p++ = '/';
      while(read(fd, &de, sizeof(de)) == sizeof(de)){
        if(de.inum == 0)
          continue;
        if(strcmp(de.name, ".") == 0 || strcmp(de.name, "..") == 0)
          continue;
        memmove(p, de.name, DIRSIZ);
        p[DIRSIZ] = 0;
        find(buf, name, doexec, cmd);

      }
    }
  }
  close(fd);
}

int
main(int argc, char *argv[])
{
  if(argc < 3){
    fprintf(2, "usage: find <path> <name> [-exec cmd...]\n");
    exit(1);
  }

  int doexec = 0;
  char *cmd[MAXARG];

  if(argc > 3 && strcmp(argv[3], "-exec") == 0){
    doexec = 1;
    int i;
    for(i = 4; i < argc; i++)
      cmd[i-4] = argv[i];
    cmd[argc-4] = 0;
  }

  find(argv[1], argv[2], doexec, cmd);
  exit(0);
}




int matchhere(char*, char*);
int matchstar(int, char*, char*);

int
match(char *re, char *text)
{
  if(re[0] == '^')
    return matchhere(re+1, text);
  do{  // must look at empty string
    if(matchhere(re, text))
      return 1;
  }while(*text++ != '\0');
  return 0;
}
// matchhere: search for re at beginning of text
int matchhere(char *re, char *text)
{
  if(re[0] == '\0')
    return 1;
  if(re[1] == '*')
    return matchstar(re[0], re+2, text);
  if(re[0] == '$' && re[1] == '\0')
    return *text == '\0';
  if(*text!='\0' && (re[0]=='.' || re[0]==*text))
    return matchhere(re+1, text+1);
  return 0;
}

// matchstar: search for c*re at beginning of text
int matchstar(int c, char *re, char *text)
{
  do{  // a * matches zero or more instances
    if(matchhere(re, text))
      return 1;
  }while(*text!='\0' && (*text++==c || c=='.'));
  return 0;
}
