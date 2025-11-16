#include "kernel/param.h"
#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"

int
main(int argc, char *argv[])
{
  int pid;
  int mask;
  char *path;

  if(argc < 4){
    fprintf(2, "Usage: sandbox <mask> <path> command...\n");
    exit(1);
  }

  mask = atoi(argv[1]);
  path = argv[2];

  pid = fork();
  if(pid < 0){
    fprintf(2, "sandbox: fork failed\n");
    exit(1);
  }

  if(pid == 0){
    // child
    if(interpose(mask, path) < 0){
      fprintf(2, "sandbox: interpose failed\n");
      exit(1);
    }
    exec(argv[3], argv+3);
    fprintf(2, "sandbox: exec %s failed\n", argv[3]);
    exit(1);
  }

  // parent
  wait(0);
  exit(0);
}
