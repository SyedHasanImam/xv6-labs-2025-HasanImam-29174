#include "types.h"
#include "riscv.h"
#include "defs.h"
#include "param.h"
#include "memlayout.h"
#include "spinlock.h"
#include "proc.h"
#include "vm.h"

uint64
sys_exit(void)
{
  int n;
  argint(0, &n);
  kexit(n);
  return 0;  // not reached
}

uint64
sys_getpid(void)
{
  return myproc()->pid;
}

uint64
sys_fork(void)
{
  return kfork();
}

uint64
sys_wait(void)
{
  uint64 p;
  argaddr(0, &p);
  return kwait(p);
}

uint64
sys_sbrk(void)
{
  uint64 addr;
  int t;
  int n;

  argint(0, &n);
  argint(1, &t);
  addr = myproc()->sz;

  if(t == SBRK_EAGER || n < 0) {
    if(growproc(n) < 0) {
      return -1;
    }
  } else {
    // Lazily allocate memory for this process: increase its memory
    // size but don't allocate memory. If the processes uses the
    // memory, vmfault() will allocate it.
    if(addr + n < addr)
      return -1;
    myproc()->sz += n;
  }
  return addr;
}

uint64
sys_pause(void)
{
  int n;
  uint ticks0;

  argint(0, &n);
  if(n < 0)
    n = 0;
  backtrace();
  acquire(&tickslock);
  ticks0 = ticks;
  while(ticks - ticks0 < n){
    if(killed(myproc())){
      release(&tickslock);
      return -1;
    }
    sleep(&ticks, &tickslock);
  }
  release(&tickslock);
  return 0;
}

uint64
sys_kill(void)
{
  int pid;

  argint(0, &pid);
  return kkill(pid);
}

// return how many clock tick interrupts have occurred
// since start.
uint64
sys_uptime(void)
{
  uint xticks;

  acquire(&tickslock);
  xticks = ticks;
  release(&tickslock);
  return xticks;
}

uint64
sys_sigalarm(void)
{
  int ticks;
  uint64 handler;
  struct proc *p = myproc();

  argint(0, &ticks);
  argaddr(1, &handler);

  p->alarm_interval = ticks;
  p->alarm_handler = (void (*)())handler;
  p->alarm_ticks = 0;

  return 0;
}

uint64
sys_sigreturn(void)
{
  struct proc *p = myproc();
  p->trapframe->epc = p->trapframe_backup.epc;
  p->trapframe->sp = p->trapframe_backup.sp;
  p->trapframe->ra = p->trapframe_backup.ra;
  p->trapframe->gp = p->trapframe_backup.gp;
  p->trapframe->tp = p->trapframe_backup.tp;
  p->trapframe->t0 = p->trapframe_backup.t0;
  p->trapframe->t1 = p->trapframe_backup.t1;
  p->trapframe->t2 = p->trapframe_backup.t2;
  p->trapframe->s0 = p->trapframe_backup.s0;
  p->trapframe->s1 = p->trapframe_backup.s1;
  p->trapframe->a0 = p->trapframe_backup.a0;
  p->trapframe->a1 = p->trapframe_backup.a1;
  p->trapframe->a2 = p->trapframe_backup.a2;
  p->trapframe->a3 = p->trapframe_backup.a3;
  p->trapframe->a4 = p->trapframe_backup.a4;
  p->trapframe->a5 = p->trapframe_backup.a5;
  p->trapframe->a6 = p->trapframe_backup.a6;
  p->trapframe->a7 = p->trapframe_backup.a7;
  p->trapframe->s2 = p->trapframe_backup.s2;
  p->trapframe->s3 = p->trapframe_backup.s3;
  p->trapframe->s4 = p->trapframe_backup.s4;
  p->trapframe->s5 = p->trapframe_backup.s5;
  p->trapframe->s6 = p->trapframe_backup.s6;
  p->trapframe->s7 = p->trapframe_backup.s7;
  p->trapframe->s8 = p->trapframe_backup.s8;
  p->trapframe->s9 = p->trapframe_backup.s9;
  p->trapframe->s10 = p->trapframe_backup.s10;
  p->trapframe->s11 = p->trapframe_backup.s11;
  p->trapframe->t3 = p->trapframe_backup.t3;
  p->trapframe->t4 = p->trapframe_backup.t4;
  p->trapframe->t5 = p->trapframe_backup.t5;
  p->trapframe->t6 = p->trapframe_backup.t6;
  
  p->alarm_ticks = 0; // Re-arm alarm

  return p->trapframe->a0;
}
