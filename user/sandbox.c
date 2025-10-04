#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"

int
main(int argc, char *argv[])
{
  if (argc < 4) {
    fprintf(2, "Usage: sandbox mask allowed_path command [args...]\n");
    exit(1);
  }

  int mask = atoi(argv[1]);
  char *allowed_path = argv[2];

  // call sys_interpose
  if (interpose(mask, allowed_path) < 0) {
    fprintf(2, "sandbox: interpose failed\n");
    exit(1);
  }

  // command starts at argv[3]
  exec(argv[3], &argv[3]);

  // exec failed
  fprintf(2, "sandbox: exec %s failed\n", argv[3]);
  exit(1);
}

