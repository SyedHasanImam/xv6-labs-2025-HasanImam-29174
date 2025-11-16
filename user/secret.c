#include "kernel/param.h"
#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"

int
main(int argc, char *argv[])
{
  char *secret;
  int i;

  if(argc < 2){
    fprintf(2, "Usage: secret <string>\n");
    exit(1);
  }

  secret = sbrk(4096);
  if(secret == (char*)-1){
    fprintf(2, "secret: sbrk failed\n");
    exit(1);
  }

  // Write the secret multiple times to increase chance of recovery
  for(i = 0; i < 8; i++){
    strcpy(secret + i * 512, argv[1]);
  }

  exit(0);
}
