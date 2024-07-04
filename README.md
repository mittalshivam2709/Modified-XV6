# Modified version of xv6 Operating System


### Introduction 

- xv6 is a simplified operating system developed at MIT. 

- xv6 can be thougth of as a "toy" operating system that helps people, especially students studying computer science, learn how real operating systems function. xv6 provides the basic interfaces introduced in the Unix OS and also mimics Unix’s internal design. By studying xv6, students can grasp fundamental concepts like process management, memory management, file systems, and how different parts of an operating system interact with each other.

- It is a re-implementation of Dennis Ritchie’s and Ken Thompson’s Unix version 6 (v6). xv6 loosely follows the structure and style of v6, but is implemented for a modern RISC-V multiprocessor using ANSI C. 

# Specifications

# System Calls 

System calls provide an interface to the user programs to communicate requests to the operating systems. 
The implemented system calls are the following.

### 1. getreadcount

The system call returns the value of a counter which is incremented every time any process calls the read() system call.

Following is the function signature for the call:
```bash
int getreadcount(void)
```

NOTE: Instructions to test the code are given in the initial_xv6/README.md file.

### 2. sigalarm and sigreturn

sigalarm is a feature to xv6 that periodically alerts a process as it uses CPU time. This might be useful for compute-bound processes that want to limit how much CPU time they chew up, or for processes that want to compute but also want to take some periodic action. More generally, it is a primitive form of user-level interrupt/fault handlers. 

```bash
sigalarm(interval, handler)
```

If an application calls alarm(n, fn) , then after every n  ”ticks” of CPU time that the program consumes, the kernel will cause application function fn to be called. When fn returns, the application will resume where it left off.

We maintain a track of ticks which have already passed by completed_clock_val and to store the value passed by syscall ticks. We also store the handler function address. After the handler function returns, time interrupt can still occur. Hence, we store the values in all the variables. The variables we store to trapframe are overwritten when the function expires, thus we maintain a cpy_trapframe to store the values. 

```bash
sigreturn()
```

It resets the process state to before the handler was called. This system call is made at the end of the handler so the process can resume where it left off.

NOTE: Instructions to test the code are given in the initial_xv6/README.md file.

### 3. Copy on Write Fork

In xv6 the fork system call creates a duplicate process of the parent process and also copies the memory content of the parent process into the child process. This results in inefficient usage of memory since the child may only read from memory.

The idea behind a copy-on-write is that when a parent process creates a child process then both of these processes initially will share the same pages in memory and these shared pages will be marked as copy-on-write which means that if any of these processes will try to modify the shared pages then only a copy of these pages will be created and the modifications will be done on the copy of pages by that process and thus not affecting the other process.

The basic plan in COW fork is for the parent and child to initially share all physical pages, but to map them read-only. Thus, when the child or parent executes a store instruction, the RISC-V CPU raises a page-fault exception. In response to this exception, the kernel makes a copy of the page that contains the faulted address. It maps one copy read/write in the child’s address space and the other copy read/write in the parent’s address space. After updating the page tables, the kernel resumes the faulting process at the instruction that caused the fault. Because the kernel has updated the relevant PTE to allow writes, the faulting instruction will now execute without a fault.


# Scheduling Algorithms

The default scheduler of xv6 is round-robin-based. Three other scheduling policies are implemented in this OS. The kernel shall only use one scheduling policy which will be declared at compile time, with default being round robin in case none is specified.

The makefile is modified to support the SCHEDULER macro to compile the specified scheduling algorithm. The following flags are used for compilation:-

```bash
- First Come First Serve = FCFS
- Multilevel Feedback Queue = MLFQ
- Priority Based Scheduling = PBS
```

### 1. FCFS ( First Come First Serve ) 

FCFS selects the process with the lowest creation time (creation time refers to the tick number when the process was created). The process will run until it no longer needs CPU time.


Edit the struct proc (used for storing per-process information) in kernel/proc.h to store extra information about the process.
Modify the allocproc() function to set up values when the process starts. (see kernel/proc.h)
Use pre-processor directives to declare the alternate scheduling policy in scheduler() in kernel/proc.h.
Disable the preemption of the process after the clock interrupts in kernel/trap.c

### 2. MLFQ (Multi Level Feedback Queue) 

- Preemptive MLFQ scheduler is implemented that allows processes to move between different priority queues based on their behavior and CPU bursts.

- If a process uses too much CPU time, it is pushed to a lower priority queue, leaving I/O bound and interactive processes in the higher priority queues.

- Four priority queues are created, giving the highest priority to queue number 0 and lowest priority to queue number 3. The time-slice are as follows:
    - For priority 0: 1 timer tick.

    - For priority 1: 3 timer ticks.

    - For priority 2: 9 timer ticks.

    - For priority 3: 15 timer ticks.

    NOTE: Here tick refers to the clock interrupt timer.

- Synopsis for the scheduler:-

    - On the initiation of a process, it is pushed to the end of the highest priority queue. The processes that are in the highest priority queue that is not empty are run first.

        ```
        Example:
        Initial Condition: A process is running in queue number 2 and there are no processes in both queues 1 and 0.

        Now if another process enters in queue 0, then the current running process (residing in queue number 2) is  preempted and the process in queue 0 is be allocated the CPU.(The kernel only preempts the process when it gets control of the hardware which is at the end of each tick so this condition is assumed here.)
        ```
- When the process completes, it leaves the system.

- If a process voluntarily relinquishes control of the CPU(eg. For doing I/O), it leaves the queuing network, and when the process becomes ready again after the I/O, it is inserted at the tail of the 
same queue, from which it is relinquished earlier.

- A round-robin scheduler is used for processes at the lowest priority queue. 

- To prevent starvation, aging is implemented :
    If the wait time of a process in a priority queue (other than priority 0) exceeds a given limit, their priority is increased and they are pushed to the next higher priority queue. 
    The wait time is reset to 0 whenever a process gets selected by the scheduler or if a change in the queue takes place (because of aging).

-  Procdump:
This is useful for debugging (coded in kernel/proc.c ). It prints a list of processes to the console when a user types Ctrl-P on the console. This functionality can be modified to print the state of the running process and display the other relevant information on the console.
procdump function can be used to print the current status of the processes and check whether the processes are scheduled according to the logic. 

Note:  ```schedulertest``` command is used for testing, it provides a positive status if the implementation works fine.


### 3. Modified Priority Based Scheduler 

PBS is a preemptive priority-based scheduler that selects the process with the highest priority for execution. In case two or more processes have the same priority, the number of times the process has been scheduled is used to break the tie. If the tie remains, the start-time of the process is used to break the tie(processes with lower start times are be scheduled further).

There are two types of priorities.

- The Static Priority of a process (SP) can be in the range [0,100], a smaller value will represent higher priority. The default priority of a process is set to 50.

- The Dynamic Priority (DP) of a process depends on its Static Priority and RBI (recent behaviour index).

The RBI (Recent Behaviour Index) of a process measures its recent behavior and is used to adjust its dynamic priority. It is a weighted sum of three factors: Running Time (RTime), Sleeping Time (STime), and Waiting Time (WTime). The default value of RBI is 25.

- Definition of the variables:
    - RTime: The total time the process has been running since it was last scheduled.
    - STime: The total time the process has spent sleeping (i.e., blocked and not using CPU time) since it was last scheduled.
    - WTime: The total time the process has spent in the ready queue waiting to be scheduled.
    - $$ RBI=max{\left (Int\left({{3*RTime - STime - WTime}\over RTime + WTime+ STime +1}*50\right),0\right)} $$

Dynamic Priority (DP) is used to schedule processes.

To change the Static Priority new system call set_priority() is added.
```bash
  int set_priority(int pid, int new_priority)
```
The system call returns the old Static Priority of the process. In case the priority of the process increases(the value is lower than before), then rescheduling is done. Note that calling this system call will also reset the Recent Behaviour Index (RBI) of the process to 25 as well.

A user program setpriority is implemented , which uses the above system call to change the priority. And takes the syscall arguments as command-line arguments.
```bash
setpriority pid priority
```


The analysis of rtimes and wtimes for different scheduling policies is as follows:-
Peformance Comparison:

The shown rtime and wtime are with CPUS=1 for the sake of comparison

Round Robin :

```bash
Average rtime 15,  wtime 160
```
FCFS : 
```bash
Average rtime 34,  wtime 135
```
MLFQ : 
```bash
Average rtime 16,  wtime 164
```
PBS :
```bash
Average rtime 15,  wtime 127
```

In roundrobin the process are executed in a cyclic order one by one and the cpu performs a context switch after each tick whereas in fcfs the cpu completes executing one process and then goes to the other, this is why the response time is more in fcfs. For mlfq  we can say that every time a new process comes it is added to the end of the first queue in which the time slice is one tick. hence the first time when the process gets the cpu control is similar to round robin hence the response time is similar to that of round robin.

the graph my_plot.png shows that the cpu bound processes are in queues 1,2,3 and 4 and their priority is being reduced timely. The file test.txt contains the data which has been used for creating the graph.
I have not handled aging however it can be done by using a for loop at the beginning of the scheduler function. 