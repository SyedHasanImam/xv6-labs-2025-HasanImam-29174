#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"

int
main(int argc, char *argv[])
{
  if (argc < 3) {
    fprintf(2, "Usage: sandbox mask [-] command [args...]\n");
    exit(1);
  }

  int mask = atoi(argv[1]);

  // If the second arg is "-" skip it
  int cmd_index = 2;
  if (strcmp(argv[2], "-") == 0) {
    if (argc < 4) {
      fprintf(2, "Usage: sandbox mask [-] command [args...]\n");
      exit(1);
    }
    cmd_index = 3;
  }

  if (interpose(mask) < 0) {
    fprintf(2, "sandbox: interpose failed\n");
    exit(1);
  }

  exec(argv[cmd_index], &argv[cmd_index]);

  // exec failed
  fprintf(2, "sandbox: exec %s failed\n", argv[cmd_index]);
  exit(1);
}

