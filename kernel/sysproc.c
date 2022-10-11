#include "types.h"
#include "riscv.h"
#include "defs.h"
#include "param.h"
#include "memlayout.h"
#include "spinlock.h"
#include "proc.h"

uint64
sys_exit(void)
{
  int n;
  argint(0, &n);
  exit(n);
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
  return fork();
}

uint64
sys_wait(void)
{
  uint64 p;
  argaddr(0, &p);
  return wait(p);
}

uint64
sys_sbrk(void)
{
  uint64 addr;
  int n;

  argint(0, &n);
  addr = myproc()->sz;
  if(growproc(n) < 0)
    return -1;
  return addr;
}

uint64
sys_sleep(void)
{
  int n;
  uint ticks0;

  argint(0, &n);
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
  return kill(pid);
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
sys_strace(void)
{
  int mask = 0;
  argint(0, &mask);
  myproc()->mask = mask;
  return 0;
}

uint64 sys_sigalarm(void)
{
  uint64 addr;
  int ticks;

  printf("\nsys_sigalarm\n");

  argint(0, &ticks);
  argaddr(0, &addr);

  printf("ticks: %d\n", ticks);
  printf("addr: %p\n", addr);
  myproc()->ticks = ticks;
  myproc()->handler = addr;

  printf("myproc()->ticks: %d\n", myproc()->ticks);
  printf("myproc()->handler: %p\n", myproc()->handler);

  return 0;
}

uint64 sys_sigreturn(void)
{
  struct proc *p = myproc();
  memmove(p->trapframe, p->alarm_tf, PGSIZE);

  kfree(p->alarm_tf);
  p->alarm_tf = 0;
  p->alarm_on = 0;
  p->cur_ticks = 0;
  return p->trapframe->a0;
}

uint64 sys_settickets(void)
{
  struct proc *p = myproc();
  int tickets;
  argint(0, &tickets);
  p->tickets = tickets;
  return tickets;
}