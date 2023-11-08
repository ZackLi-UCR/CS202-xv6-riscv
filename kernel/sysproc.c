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

// hello syscall definition
uint64 sys_hello(void)
{
	int n;
	argint(0, &n);
	print_hello(n);
	return 0;
}

// sysinfo syscall definition
extern int total_syscall_count;
uint64 sys_sysinfo(void)
{
	int n;
	argint(0, &n);
	
	if(n == 0){
		int processes_count = get_active_process();
		return processes_count;
	} else if(n == 1){
 		int syscall_count = get_total_syscall() - 1;
		return syscall_count;
	} else if(n == 2){
		int pages_count = get_free_memory_page();
		return pages_count;
	} else {
		return -1;
	}
}

// procinfo syscall definition
#define NULL ((void*)0)
uint64 sys_procinfo(void)
{
	uint64 pinfo;
	argaddr(0, &pinfo);
	struct proc *p = myproc();
	int info[3];
	info[0] = p->parent->pid;
	info[1] = p->syscall_count - 1;
	info[2] = (p->sz+PGSIZE-1)/PGSIZE;
	return copyout(p->pagetable, pinfo, (void*)info, sizeof(int)*3);
}

// sched_statistics syscall definition
uint64 sys_sched_statistics(void)
{
  print_sched_statistics();
  return 0;
}

// set schedule tickets
uint64 sys_sched_tickets(void)
{
  int n;
  argint(0, &n);
  struct proc *p = myproc();
  acquire(&p->lock);
  p->tickets = n;
  p->stride = (int)(STRIDEK / n);
  p->pass = p->stride;
  release(&p->lock);
  return 0;
}