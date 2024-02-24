#include "types.h"
#include "param.h"
#include "memlayout.h"
#include "riscv.h"
#include "spinlock.h"
#include "proc.h"
#include "defs.h"
// #define MLFQ
#ifdef MLFQ
struct proc* queue1[1000];
struct proc* queue2[1000];
struct proc* queue3[1000];
struct proc* queue4[1000];
int idx1=-1;
int idx2=-1;
int idx3=-1;
int idx4=-1;

#endif
struct cpu cpus[NCPU];

struct proc proc[NPROC];

struct proc *initproc;

int nextpid = 1;
struct spinlock pid_lock;

extern void forkret(void);
static void freeproc(struct proc *p);

extern char trampoline[]; // trampoline.S

// helps ensure that wakeups of wait()ing
// parents are not lost. helps obey the
// memory model when using p->parent.
// must be acquired before any p->lock.
struct spinlock wait_lock;

// Allocate a page for each process's kernel stack.
// Map it high in memory, followed by an invalid
// guard page.
void proc_mapstacks(pagetable_t kpgtbl)
{
  struct proc *p;

  for (p = proc; p < &proc[NPROC]; p++)
  {
    char *pa = kalloc();
    if (pa == 0)
      panic("kalloc");
    uint64 va = KSTACK((int)(p - proc));
    kvmmap(kpgtbl, va, (uint64)pa, PGSIZE, PTE_R | PTE_W);
  }
}

// initialize the proc table.
void procinit(void)
{
  struct proc *p;

  initlock(&pid_lock, "nextpid");
  initlock(&wait_lock, "wait_lock");
  for (p = proc; p < &proc[NPROC]; p++)
  {
    initlock(&p->lock, "proc");
    p->state = UNUSED;
    p->kstack = KSTACK((int)(p - proc));
  }
}

// Must be called with interrupts disabled,
// to prevent race with process being moved
// to a different CPU.
int cpuid()
{
  int id = r_tp();
  return id;
}

// Return this CPU's cpu struct.
// Interrupts must be disabled.
struct cpu *
mycpu(void)
{
  int id = cpuid();
  struct cpu *c = &cpus[id];
  return c;
}

// Return the current struct proc *, or zero if none.
struct proc *
myproc(void)
{
  push_off();
  struct cpu *c = mycpu();
  struct proc *p = c->proc;
  pop_off();
  return p;
}

int allocpid()
{
  int pid;

  acquire(&pid_lock);
  pid = nextpid;
  nextpid = nextpid + 1;
  release(&pid_lock);

  return pid;
}

// Look in the process table for an UNUSED proc.
// If found, initialize state required to run in the kernel,
// and return with p->lock held.
// If there are no free procs, or a memory allocation fails, return 0.
static struct proc *
allocproc(void)
{
  struct proc *p;

  for (p = proc; p < &proc[NPROC]; p++)
  {
    acquire(&p->lock);
    if (p->state == UNUSED)
    {
      goto found;
    }
    else
    {
      release(&p->lock);
    }
  }
  return 0;

found:
  #ifdef MLFQ

    // printf("here");
    queue1[++idx1]=p;
    p->qidx=idx1;
  #endif
  p->ticks=0;
  p->numreads=0;
  p->pid = allocpid();
  p->state = USED;
  p->queuenum=1;
  // Allocate a trapframe page.
  if ((p->trapframe = (struct trapframe *)kalloc()) == 0)
  {
    freeproc(p);
    release(&p->lock);
    return 0;
  }

  // An empty user page table.
  p->pagetable = proc_pagetable(p);
  if (p->pagetable == 0)
  {
    freeproc(p);
    release(&p->lock);
    return 0;
  }

  // Set up new context to start executing at forkret,
  // which returns to user space.
  memset(&p->context, 0, sizeof(p->context));
  p->context.ra = (uint64)forkret;
  p->context.sp = p->kstack + PGSIZE;
  p->rtime = 0;
  p->etime = 0;
  p->wtime=0;
  
  p->ctime = ticks;
  return p;
}
static void
freeproc(struct proc *p)
{
  if (p->trapframe)
    kfree((void *)p->trapframe);
  p->trapframe = 0;
  if (p->pagetable)
    proc_freepagetable(p->pagetable, p->sz);
  p->pagetable = 0;
  p->sz = 0;
  p->pid = 0;
  p->parent = 0;
  p->name[0] = 0;
  p->chan = 0;
  p->killed = 0;
  p->xstate = 0;
  p->state = UNUSED;
}

// Create a user page table for a given process, with no user memory,
// but with trampoline and trapframe pages.
pagetable_t
proc_pagetable(struct proc *p)
{
  pagetable_t pagetable;

  // An empty page table.
  pagetable = uvmcreate();
  if (pagetable == 0)
    return 0;

  // map the trampoline code (for system call return)
  // at the highest user virtual address.
  // only the supervisor uses it, on the way
  // to/from user space, so not PTE_U.
  if (mappages(pagetable, TRAMPOLINE, PGSIZE,
               (uint64)trampoline, PTE_R | PTE_X) < 0)
  {
    uvmfree(pagetable, 0);
    return 0;
  }

  // map the trapframe page just below the trampoline page, for
  // trampoline.S.
  if (mappages(pagetable, TRAPFRAME, PGSIZE,
               (uint64)(p->trapframe), PTE_R | PTE_W) < 0)
  {
    uvmunmap(pagetable, TRAMPOLINE, 1, 0);
    uvmfree(pagetable, 0);
    return 0;
  }

  return pagetable;
}

// Free a process's page table, and free the
// physical memory it refers to.
void proc_freepagetable(pagetable_t pagetable, uint64 sz)
{
  uvmunmap(pagetable, TRAMPOLINE, 1, 0);
  uvmunmap(pagetable, TRAPFRAME, 1, 0);
  uvmfree(pagetable, sz);
}

// a user program that calls exec("/init")
// assembled from ../user/initcode.S
// od -t xC ../user/initcode
uchar initcode[] = {
    0x17, 0x05, 0x00, 0x00, 0x13, 0x05, 0x45, 0x02,
    0x97, 0x05, 0x00, 0x00, 0x93, 0x85, 0x35, 0x02,
    0x93, 0x08, 0x70, 0x00, 0x73, 0x00, 0x00, 0x00,
    0x93, 0x08, 0x20, 0x00, 0x73, 0x00, 0x00, 0x00,
    0xef, 0xf0, 0x9f, 0xff, 0x2f, 0x69, 0x6e, 0x69,
    0x74, 0x00, 0x00, 0x24, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00};

// Set up first user process.
void userinit(void)
{
  struct proc *p;

  p = allocproc();
  initproc = p;

  // allocate one user page and copy initcode's instructions
  // and data into it.
  uvmfirst(p->pagetable, initcode, sizeof(initcode));
  p->sz = PGSIZE;

  // prepare for the very first "return" from kernel to user.
  p->trapframe->epc = 0;     // user program counter
  p->trapframe->sp = PGSIZE; // user stack pointer

  safestrcpy(p->name, "initcode", sizeof(p->name));
  p->cwd = namei("/");

  p->state = RUNNABLE;

  release(&p->lock);
}

// Grow or shrink user memory by n bytes.
// Return 0 on success, -1 on failure.
int growproc(int n)
{
  uint64 sz;
  struct proc *p = myproc();

  sz = p->sz;
  if (n > 0)
  {
    if ((sz = uvmalloc(p->pagetable, sz, sz + n, PTE_W)) == 0)
    {
      return -1;
    }
  }
  else if (n < 0)
  {
    sz = uvmdealloc(p->pagetable, sz, sz + n);
  }
  p->sz = sz;
  return 0;
}

// Create a new process, copying the parent.
// Sets up child kernel stack to return as if from fork() system call.
int fork(void)
{
  int i, pid;
  struct proc *np;
  struct proc *p = myproc();

  // Allocate process.
  if ((np = allocproc()) == 0)
  {
    return -1;
  }

  // Copy user memory from parent to child.
  if (uvmcopy(p->pagetable, np->pagetable, p->sz) < 0)
  {
    freeproc(np);
    release(&np->lock);
    return -1;
  }
  np->sz = p->sz;

  // copy saved user registers.
  *(np->trapframe) = *(p->trapframe);

  // Cause fork to return 0 in the child.
  np->trapframe->a0 = 0;

  // increment reference counts on open file descriptors.
  for (i = 0; i < NOFILE; i++)
    if (p->ofile[i])
      np->ofile[i] = filedup(p->ofile[i]);
  np->cwd = idup(p->cwd);

  safestrcpy(np->name, p->name, sizeof(p->name));

  pid = np->pid;

  release(&np->lock);

  acquire(&wait_lock);
  np->parent = p;
  release(&wait_lock);

  acquire(&np->lock);
  np->state = RUNNABLE;
  release(&np->lock);

  return pid;
}

// Pass p's abandoned children to init.
// Caller must hold wait_lock.
void reparent(struct proc *p)
{
  struct proc *pp;

  for (pp = proc; pp < &proc[NPROC]; pp++)
  {
    if (pp->parent == p)
    {
      pp->parent = initproc;
      wakeup(initproc);
    }
  }
}

// Exit the current process.  Does not return.
// An exited process remains in the zombie state
// until its parent calls wait().
void exit(int status)
{
  struct proc *p = myproc();

  if (p == initproc)
    panic("init exiting");

  // Close all open files.
  for (int fd = 0; fd < NOFILE; fd++)
  {
    if (p->ofile[fd])
    {
      struct file *f = p->ofile[fd];
      fileclose(f);
      p->ofile[fd] = 0;
    }
  }
  begin_op();
  iput(p->cwd);
  end_op();
  p->cwd = 0;

  acquire(&wait_lock);

  // Give any children to init.
  reparent(p);

  // Parent might be sleeping in wait().
  wakeup(p->parent);

  acquire(&p->lock);

  p->xstate = status;
  p->state = ZOMBIE;
  p->etime = ticks;

  release(&wait_lock);

  // Jump into the scheduler, never to return.
  sched();
  panic("zombie exit");
}

// Wait for a child process to exit and return its pid.
// Return -1 if this process has no children.
int wait(uint64 addr)
{
  struct proc *pp;
  int havekids, pid;
  struct proc *p = myproc();

  acquire(&wait_lock);

  for (;;)
  {
    // Scan through table looking for exited children.
    havekids = 0;
    for (pp = proc; pp < &proc[NPROC]; pp++)
    {
      if (pp->parent == p)
      {
        // make sure the child isn't still in exit() or swtch().
        acquire(&pp->lock);

        havekids = 1;
        if (pp->state == ZOMBIE)
        {
          // Found one.
          pid = pp->pid;
          if (addr != 0 && copyout(p->pagetable, addr, (char *)&pp->xstate,
                                   sizeof(pp->xstate)) < 0)
          {
            release(&pp->lock);
            release(&wait_lock);
            return -1;
          }
          freeproc(pp);
          release(&pp->lock);
          release(&wait_lock);
          return pid;
        }
        release(&pp->lock);
      }
    }

    // No point waiting if we don't have any children.
    if (!havekids || killed(p))
    {
      release(&wait_lock);
      return -1;
    }

    // Wait for a child to exit.
    sleep(p, &wait_lock); // DOC: wait-sleep
  }
}

// Per-CPU process scheduler.
// Each CPU calls scheduler() after setting itself up.
// Scheduler never returns.  It loops, doing:
//  - choose a process to run.
//  - swtch to start running that process.
//  - eventually that process transfers control
//    via swtch back to the scheduler.
void scheduler(void)
{
  struct proc *p;
  struct cpu *c = mycpu();
  c->proc = 0;
  for (;;)
  {
    // Avoid deadlock by ensuring that devices can interrupt.
    intr_on();
    // following is for aging

    // for (p = proc; p < &proc[NPROC]; p++)
    // {
    //     acquire(&p->lock);
    //     if(p->wtime >= 100){
    //       p->priority--;
          
    //     }
    // }  
    #ifdef roundrobin
    
      for (p = proc; p < &proc[NPROC]; p++)
      { 
        acquire(&p->lock);
        if (p->state == RUNNABLE)
        {
          // Switch to chosen process.  It is the process's job
          // to release its lock and then reacquire it
          // before jumping back to us.
          p->state = RUNNING;
          c->proc = p;
          swtch(&c->context, &p->context);
  
          // Process is done running for now.
          // It should have changed its p->state before coming back.
          c->proc = 0;
        }
        release(&p->lock);
      }
    #endif
    #ifdef FCFS
        // printf("here");
      struct proc* minm=0;
      // for (p = proc; p < &proc[NPROC]; p++){
      //   acquire(&p->lock);
      // }
      // minm=(struct proc*)kalloc();
      // acquire(&minm->lock);
      for (p = proc; p < &proc[NPROC]; p++)
      {
        acquire(&p->lock);
        if (p->state != RUNNABLE){
          release(&p->lock);
          continue;
        }
        
          if(minm==0){
            // printf("here");
            minm=p;
          }
          else{
            acquire(&minm->lock);
                if(p->ctime<minm->ctime){
                  minm=p;
                }
            release(&minm->lock);
          }
        // printf("%d",minm->ctime);
        release(&p->lock);
      }

      // printf("%d",minm->ctime);
      // for (p = proc; p < &proc[NPROC]; p++){
      //   release(&p->lock);
      // }
      if(minm==0){
        continue;
      }
      acquire(&minm->lock);
      if(minm->state==RUNNABLE){
        // Switch to chosen process.  It is the process's job
        // to release its lock and then reacquire it
        // before jumping back to us.
        minm->state = RUNNING;
        c->proc = minm;
        swtch(&c->context, &minm->context);
        // Process is done running for now.
        // It should have changed its p->state before coming back.
        c->proc = 0;
      }
      release(&minm->lock);
    #endif
    #ifdef MLFQ
      for (p = proc; p < &proc[NPROC]; p++){
          acquire(&p->lock);
          if(p->state==RUNNABLE || p->state==RUNNING){
            int val=p->queuenum;
            int pid=p->pid;
            release(&p->lock);
            if(val==1){
              int f=0;
              for (int i = 0; i <= idx1; i++){
                acquire(&queue1[i]->lock);
                if(queue1[i]->pid==pid){
                  f=1;
                  release(&queue1[i]->lock);
                  break;
                }
                release(&queue1[i]->lock);
              }
              if(f==0){
                queue1[++idx1]=p;
              }
          }
          else if(val==2){
            int f=0;
            for (int i = 0; i <= idx2; i++){
              acquire(&queue2[i]->lock);
              if(queue2[i]->pid==pid){
                f=1;
                release(&queue2[i]->lock);
                break;
              }
              release(&queue2[i]->lock);
            }
            if(f==0){
              queue2[++idx2]=p;
            }
          }
          else if(val==3){
            int f=0;
            for (int i = 0; i <= idx3; i++){
              acquire(&queue3[i]->lock);
              if(queue3[i]->pid==pid){
                f=1;
                release(&queue3[i]->lock);
                break;
              }
              release(&queue3[i]->lock);
            }
            if(f==0){
              queue3[++idx3]=p;
            }
          }
          else if(val==4){
            int f=0;
            for (int i = 0; i <= idx4; i++){
              acquire(&queue4[i]->lock);
              if(queue4[i]->pid==pid){
                f=1;
                release(&queue4[i]->lock);
                break;
              }
              release(&queue4[i]->lock);
            }
            if(f==0){
              queue4[++idx4]=p;
            }
          }
        }
        else{
          release(&p->lock);
        }
      }
      // for (p = proc; p < &proc[NPROC]; p++){
      //     acquire(&p->lock);
      //     if(p->state!=RUNNABLE && p->state !=UNUSED){
      //       // printf("inside this remove loop for %s for state %d ",p->name,p->state);
      //         int qnum=p->queuenum;
      //         int index=p->qidx;
      //         if(index==-1){
      //           release(&p->lock);
      //           continue;
      //         }
      //         int maxidx;
      //         p->qidx=-1;
      //         if(qnum==1){
      //           maxidx=idx1;
      //           for (int i = index; i < maxidx; i++){
      //             queue1[i]=queue1[i+1];
      //           }
      //           idx1--;
      //         }
      //         else if(qnum==2){
      //           maxidx=idx2;
      //           for (int i = index; i < maxidx; i++){
      //           queue2[i]=queue2[i+1];
      //           }
      //           idx2--;
      //         }
      //         else if(qnum==3){
      //           maxidx=idx3;
      //           for (int i = index; i < maxidx; i++){
      //             queue3[i]=queue3[i+1];
      //           }
      //           idx3--;
      //         }
      //         else if(qnum==4){
      //           maxidx=idx4;
      //           for (int i = index; i < maxidx; i++){
      //             queue4[i]=queue4[i+1];
      //           }
      //           idx4--;
      //         }
      //         release(&p->lock);
      //     }
      //     else{
      //       release(&p->lock);
      //     }
      // }  
      
      // for (int i = 0; i<= idx1; i++){
      //   acquire(&queue1[i]->lock);
      //   if(queue1[i]->isfinished==1){
      //     printf("removing");
      //     release(&queue1[i]->lock);
      //     for (int j = i; j < idx1; j++){
      //       queue1[i]=queue1[i+1];
      //     }
      //     idx1--;
      //   }else{
      //     release(&queue1[i]->lock);
      //   }
      // }
      // for (int i = 0; i<= idx4; i++){
      //   acquire(&queue4[i]->lock);
      //   if(queue4[i]->isfinished==1){
      //     printf("  hatadiya  ");
      //     printf("%s ",queue4[i]->name);
      //     release(&queue4[i]->lock);
      //     for (int j = i; j < idx4; j++){
      //       queue4[i]=queue4[i+1];
      //     }
      //     idx4--;
      //   }else{
      //     release(&queue4[i]->lock);
      //   }
      // }
      int executed=0;
      // printf(" %d  ",idx1);
      if(idx1>-1){
        // printf("\n");
        // procdump();
          for (int i=0; i <= idx1; i++){
            acquire(&queue1[i]->lock);
            if(queue1[i]->state!=RUNNABLE &&queue1[i]->state !=RUNNING){
              release(&queue1[i]->lock);
              for (int j = i; j < idx1; j++){
                queue1[j]=queue1[j+1];
              }
              idx1--;
              i--;  
              // break;
              continue;
            }
            if(idx1<0){
              break;
            }
            // i have the runnable process now
            // printf("ok");
            p=queue1[i]; 
            // printf("rtime is %d for %d ",p->rtime,p->pid);
            // acquire(&p->lock);
            p->state=RUNNING;
            p->ticks++;
            p->wtime=0;
            c->proc = p;
            swtch(&c->context, &p->context);
            // Process is done running for now.
            // It should have changed its p->state before coming back.
            // printf("rtime is %d for %d \n",p->rtime,p->pid);
            c->proc = 0;
            if(p->pid ==0 ){
              for (int j = i; j < idx1; j++){
                queue1[j]=queue1[j+1];
              }
              idx1--;
              release(&p->lock);
              executed=1;
              break;
            }
            if(p->ticks >= 1){
              // printf("here");
              p->queuenum=2;
              queue2[++idx2]=p;
              p->qidx=idx2;
              // printf("%d ",idx2);
              for (int j = i; j < idx1; j++){
                queue1[j]=queue1[j+1];
              }
              idx1--;
              p->ticks=0;
              p->wtime=0;
            }
            release(&p->lock);
            executed=1;break;
          }
          if(executed==1){
            // printf("contining");
            continue;
          }
      }
      if(idx2>-1){
        // printf("\n");

      // procdump();

          for (int i=0; i <= idx2; i++){
            acquire(&queue2[i]->lock);
            if(queue2[i]->state!=RUNNABLE&&queue2[i]->state !=RUNNING){
              release(&queue2[i]->lock);
              for (int j = i; j < idx2; j++){
                queue2[j]=queue2[j+1];
              }
              idx2--;
              i--;  
              // break;
              continue;
            }
            if(idx2<0){
              break;
            }
          // printf("idx2");
            // i have the runnable process now
            p=queue2[i]; 
            // printf("rtime is %d for %d ",p->rtime,p->pid);
            // acquire(&p->lock);
            p->state=RUNNING;
            p->ticks++;
            p->wtime=0;
            c->proc = p;
            swtch(&c->context, &p->context);
            // Process is done running for now.
            // It should have changed its p->state before coming back.
            // printf("rtime is %d for %d \n",p->rtime,p->pid);
            c->proc = 0;
            if(p->pid ==0 ){
              for (int j = i; j < idx2; j++){
                queue2[j]=queue2[j+1];
              }
              idx2--;
              i--;
              // release(&p->lock);
              executed=1;
              break;
            }
            if(p->ticks >= 3){
              // printf("here");
              p->queuenum=3;
              queue3[++idx3]=p;
              p->qidx=idx3;

              for (int j = i; j < idx2; j++){
                queue2[j]=queue2[j+1];
              }
              idx2--;
              i--;
              p->ticks=0;
              p->wtime=0;
            }
            release(&p->lock);
            executed=1;break;
          }
          if(executed==1){
            continue;
          }
      }
      
      if(idx3>-1){
        // printf("\n");

        // procdump();
          for (int i=0; i <= idx3; i++){
          // printf("here");
            acquire(&queue3[i]->lock);
            if(queue3[i]->state!=RUNNABLE&&queue3[i]->state !=RUNNING){
              release(&queue3[i]->lock);
              for (int j = i; j < idx3; j++){
                queue3[j]=queue3[j+1];
              }
              idx3--;
              i--;  
              // break;
              continue;
            }
            if(idx3<0){
              break;
            }
            // i have the runnable process now
            p=queue3[i]; 
            // printf("rtime is %d for %d ",p->rtime,p->pid);
            // acquire(&p->lock);
            p->state=RUNNING;
            p->ticks++;
            p->wtime=0;
            c->proc = p;
            swtch(&c->context, &p->context);
            // Process is done running for now.
            // It should have changed its p->state before coming back.
            // printf("rtime is %d for %d \n",p->rtime,p->pid);
            c->proc = 0;
            if(p->pid ==0 ){
              for (int j = i; j < idx3; j++){
                queue3[j]=queue3[j+1];
              }
              idx3--;
              i--;
              release(&p->lock);
              executed=1;
              break;
            }
            if(p->ticks >= 9){
              // printf("here");
              p->queuenum=4;
              queue4[++idx4]=p;
              p->qidx=idx4;

              for (int j = i; j < idx3; j++){
                queue3[j]=queue3[j+1];
              }
              idx3--;
              i--;
              p->ticks=0;
              p->wtime=0;
            }
            release(&p->lock);
            executed=1;break;
          }
          if(executed==1){
            continue;
          }
      }
      if(idx4>-1){
        // printf("\n");
        // procdump();
          for (int i=0; i <= idx4; i++){
          // printf("here");
            acquire(&queue4[i]->lock);
            if(queue4[i]->state!=RUNNABLE&&queue4[i]->state !=RUNNING){
              release(&queue4[i]->lock);
              for (int j = i; j < idx4; j++){
                queue4[j]=queue4[j+1];
              }
              idx4--;
              i--;  
              // break;
              continue;
            }
            // // i have the runnable process now

            // if(idx4<0){
            //   break;
            // }
            p=queue4[i]; 
            // printf("rtime is %d for %d ",p->rtime,p->pid);
            // acquire(&p->lock);
            p->state=RUNNING;
            p->ticks++;
            p->wtime=0;
            c->proc = p;
            swtch(&c->context, &p->context);
            // Process is done running for now.
            // It should have changed its p->state before coming back.
            // printf("rtime is %d for %d \n",p->rtime,p->pid);
            c->proc = 0;
            if(p->pid ==0 ){
              for (int j = i; j < idx4; j++){
                queue4[j]=queue4[j+1];
              }
              idx4--;
              release(&p->lock);
              executed=1;
              break;
            }
            if(p->ticks >= 15){
              // printf("here");
              p->queuenum=4;
              for (int j = i; j < idx4; j++){
                queue4[j]=queue4[j+1];
              }
              queue4[idx4]=p;
              p->ticks=0;
              p->wtime=0;
            }
            release(&p->lock);
            executed=1;break;
          }
          if(executed==1){
            continue;
          }
      }
    #endif   
  }
}

// Switch to scheduler.  Must hold only p->lock
// and have changed proc->state. Saves and restores
// intena because intena is a property of this
// kernel thread, not this CPU. It should
// be proc->intena and proc->noff, but that would
// break in the few places where a lock is held but
// there's no process.
void sched(void)
{
  int intena;
  struct proc *p = myproc();

  if (!holding(&p->lock))
    panic("sched p->lock");
  if (mycpu()->noff != 1)
    panic("sched locks");
  if (p->state == RUNNING)
    panic("sched running");
  if (intr_get())
    panic("sched interruptible");
  intena = mycpu()->intena;
  swtch(&p->context, &mycpu()->context);
  mycpu()->intena = intena;
}

// Give up the CPU for one scheduling round.
void yield(void)
{
  struct proc *p = myproc();
  acquire(&p->lock);
  p->state = RUNNABLE;
  sched();
  release(&p->lock);
}

// A fork child's very first scheduling by scheduler()
// will swtch to forkret.
void forkret(void)
{
  static int first = 1;

  // Still holding p->lock from scheduler.
  release(&myproc()->lock);

  if (first)
  {
    // File system initialization must be run in the context of a
    // regular process (e.g., because it calls sleep), and thus cannot
    // be run from main().
    first = 0;
    fsinit(ROOTDEV);
  }

  usertrapret();
}

// Atomically release lock and sleep on chan.
// Reacquires lock when awakened.
void sleep(void *chan, struct spinlock *lk)
{
  struct proc *p = myproc();

  // Must acquire p->lock in order to
  // change p->state and then call sched.
  // Once we hold p->lock, we can be
  // guaranteed that we won't miss any wakeup
  // (wakeup locks p->lock),
  // so it's okay to release lk.

  acquire(&p->lock); // DOC: sleeplock1
  release(lk);

  // Go to sleep.
  p->chan = chan;
  p->state = SLEEPING;

  sched();

  // Tidy up.
  p->chan = 0;

  // Reacquire original lock.
  release(&p->lock);
  acquire(lk);
}

// Wake up all processes sleeping on chan.
// Must be called without any p->lock.

      #ifdef MLFQ
void wakeup(void *chan) 
{
  struct proc *p;

  for (p = proc; p < &proc[NPROC]; p++)
  {
    if (p != myproc())
    {
      acquire(&p->lock);
      if (p->state == SLEEPING && p->chan == chan)
      {
        p->state = RUNNABLE;
        int pid=p->pid;
        int val=p->queuenum;
        release(&p->lock);
        if(val==1){
          int f=0;
          for (int i = 0; i <= idx1; i++){
            acquire(&queue1[i]->lock);
            if(queue1[i]->pid==pid){
              f=1;
              release(&queue1[i]->lock);
              break;
            }
            release(&queue1[i]->lock);
          }
          if(f==0){
            queue1[++idx1]=p;
          }
        }
        else if(val==2){
          int f=0;
          for (int i = 0; i <= idx2; i++){
            acquire(&queue2[i]->lock);
            if(queue2[i]->pid==pid){
              f=1;
              release(&queue2[i]->lock);
              break;
            }
            release(&queue2[i]->lock);
          }
          if(f==0){
            queue2[++idx2]=p;
          }
        }
        else if(val==3){
          int f=0;
          for (int i = 0; i <= idx3; i++){
            acquire(&queue3[i]->lock);
            if(queue3[i]->pid==pid){
              f=1;
              release(&queue3[i]->lock);
              break;
            }
            release(&queue3[i]->lock);
          }
          if(f==0){
            queue3[++idx3]=p;
          }
        }
        else if(val==4){
          int f=0;
          for (int i = 0; i <= idx4; i++){
            acquire(&queue4[i]->lock);
            if(queue4[i]->pid==pid){
              f=1;
              release(&queue4[i]->lock);
              break;
            }
            release(&queue4[i]->lock);
          }
          if(f==0){
            queue4[++idx4]=p;
          }
        }
      }
      else{
      release(&p->lock);
      }
    }
  }
}
#endif

#ifndef MLFQ
void wakeup(void *chan)
{
  struct proc *p;

  for (p = proc; p < &proc[NPROC]; p++)
  {
    if (p != myproc())
    {
      acquire(&p->lock);
      if (p->state == SLEEPING && p->chan == chan)
      {
        p->state = RUNNABLE;
      }
      release(&p->lock);
    }
  }
}
#endif
// void wakeup(void *chan)
// {
//   struct proc *p;

//   for (p = proc; p < &proc[NPROC]; p++)
//   {
//     if (p != myproc())
//     {
//       acquire(&p->lock);
//       if (p->state == SLEEPING && p->chan == chan)
//       {
//         p->state = RUNNABLE;
//       #ifdef MLFQ
//       int val=p->queuenum;
//       if(val==1){
//         queue1[++idx1]=p;
//       }else if(val==2){
//         queue2[++idx2]=p;
//       }else if(val==3){
//         queue3[++idx3]=p;
//       }else if(val==4){
//         queue3[++idx4]=p;
//       }
//       #endif
//       }
//       release(&p->lock);
//     }
//   }
// }

// Kill the process with the given pid.
// The victim won't exit until it tries to return
// to user space (see usertrap() in trap.c).
int kill(int pid)
{
  struct proc *p;

  for (p = proc; p < &proc[NPROC]; p++)
  {
    acquire(&p->lock);
    if (p->pid == pid)
    {
      p->killed = 1;
      if (p->state == SLEEPING)
      {
        // Wake process from sleep().
        p->state = RUNNABLE;
      }
      release(&p->lock);
      return 0;
    }
    release(&p->lock);
  }
  return -1;
}

void setkilled(struct proc *p)
{
  acquire(&p->lock);
  p->killed = 1;
  release(&p->lock);
}

int killed(struct proc *p)
{
  int k;

  acquire(&p->lock);
  k = p->killed;
  release(&p->lock);
  return k;
}

// Copy to either a user address, or kernel address,
// depending on usr_dst.
// Returns 0 on success, -1 on error.
int either_copyout(int user_dst, uint64 dst, void *src, uint64 len)
{
  struct proc *p = myproc();
  if (user_dst){
    return copyout(p->pagetable, dst, src, len);
  }
  else
  {
    memmove((char *)dst, src, len);
    return 0;
  }
}

// Copy from either a user address, or kernel address,
// depending on usr_src.
// Returns 0 on success, -1 on error.
int either_copyin(void *dst, int user_src, uint64 src, uint64 len)
{
  struct proc *p = myproc();
  if (user_src)
  {
    return copyin(p->pagetable, dst, src, len);
  }
  else
  {
    memmove(dst, (char *)src, len);
    return 0;
  }
}

// Print a process listing to console.  For debugging.
// Runs when user types ^P on console.
// No lock to avoid wedging a stuck machine further.
void procdump(void)
{
  static char *states[] = {
      [UNUSED] "unused",
      [USED] "used",
      [SLEEPING] "sleep ",
      [RUNNABLE] "runble",
      [RUNNING] "run   ",
      [ZOMBIE] "zombie"};
  struct proc *p;
  char *state;

  printf("\n");
  for (p = proc; p < &proc[NPROC]; p++)
  {
    if (p->state == UNUSED)
      continue;
      // state = states[p->state];
    if (p->state >= 0 && p->state < NELEM(states) && states[p->state])
      state = states[p->state];
    else
      state = "???";
    #ifdef MLFQ
    printf("%d %d %d %d ",idx1,idx2,idx3,idx4);
    if(idx1==-1 && idx2==-1 && idx3==-1 && idx4==-1){
      printf("%d %d %s %s", p->pid,p->queuenum, state, p->name);
    }
    #endif 
    #ifdef FCFS
    printf("fcfs");
    #endif
    printf("%d %s %s", p->pid, state, p->name);
    printf("\n");
    // for (int i = 0; i < idx4+1; i++)
    // {
    //   acquire(&queue4[i]->lock);
    //   printf("%d ",queue4[i]->pid);
    //   release(&queue4[i]->lock);
    // }
    
    // for (int i = 0; i <= idx3; i++){
    //   printf(" %d ",queue3[i]->pid);
    // }
    
  }
}

// waitx
int waitx(uint64 addr, uint *wtime, uint *rtime)
{
  struct proc *np;
  int havekids, pid;
  struct proc *p = myproc();

  acquire(&wait_lock);

  for (;;)
  {
    // Scan through table looking for exited children.
    havekids = 0;
    for (np = proc; np < &proc[NPROC]; np++)
    {
      if (np->parent == p)
      {
        // make sure the child isn't still in exit() or swtch().
        acquire(&np->lock);

        havekids = 1;
        if (np->state == ZOMBIE)
        {
          // Found one.
          pid = np->pid;
          *rtime = np->rtime;
          *wtime = np->etime - np->ctime - np->rtime;
          if (addr != 0 && copyout(p->pagetable, addr, (char *)&np->xstate,
                                   sizeof(np->xstate)) < 0)
          {
            release(&np->lock);
            release(&wait_lock);
            return -1;
          }
          freeproc(np);
          release(&np->lock);
          release(&wait_lock);
          return pid;
        }
        release(&np->lock);
      }
    }

    // No point waiting if we don't have any children.
    if (!havekids || p->killed)
    {
      release(&wait_lock);
      return -1;
    }

    // Wait for a child to exit.
    sleep(p, &wait_lock); // DOC: wait-sleep
  }
}

void update_time()
{
  struct proc *p;
  for (p = proc; p < &proc[NPROC]; p++)
  {
    acquire(&p->lock);
    if (p->state == RUNNING)
    {
      p->rtime++;
    }
    release(&p->lock);
  }
  // for (p = proc; p < &proc[NPROC]; p++){
  //   if(p->state==RUNNING || p->state==RUNNABLE){
  //     if(p->pid>=9 && p->pid<=13){
  //       printf("%d %d %d\n",p->pid,ticks,p->queuenum);
  //     }
  //   }
  // }
}