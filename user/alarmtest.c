#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"

void periodic();

int
main(int argc, char *argv[])
{
  int i;
  printf("alarmtest starting\n");
  sigalarm(2, periodic);
  for(i = 0; i < 25*500000; i++){
    if((i % 250000) == 0)
      write(2, ".", 1);
  }
  exit(0);
}

void
periodic()
{
  printf("alarm!\n");
  sigreturn();
}
