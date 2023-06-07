
# Enhancing the OS xv6

## 1. System Calls

### Trace:

#### Input Command:
```jsx
strace mask command [args]
```

#### Changes:
**in kernel/sysproc.c:**

The mask is set to 0 to begin with. We then use `argint()` to collect the user's command line parameters and set the mask variable accordingly, before checking for command success and handling errors appropriately. We now set the process's trace mask to the value of the mask variable, which was passed as a parameter.


**kernel/proc.c:**

As an additional precaution, the trace mask must be passed down from parent to child processes when forking.

```jsx
// copy saved trace mask from parent to child
  np->systrackmask = p->systrackmask;
```

**kernel/syscall.c:**

Two arrays are required in this case. One is responsible for determining the names of the syscalls to be printed ('syscall names') and another is responsible for storing the number of valid arguments ('syscall number') that each of these syscalls accepts.

We use 'argraw(),' which returns the raw value of the i-th argument from the registers, to put the arguments in array 'arrayarg'. If both mask and 'num' are set, we proceed to the next step. When these requirements are satisfied, the command "strace" is run.

We then proceed to print:

1. The process id ⇒ `p->pid`
2. The name of the system call ⇒ using `syscall_names`
3. The decimal value of the arguments(xv6 passes arguments via registers) ⇒ using `arrayarg` which stores the values.
4. The return value of the syscall ⇒ `p->trapframe->a0` 


**user/strace.c:**

Here, we take in the parameters that were passed through the command line and handle any errors that may arise. The command line input string was also converted to an integer. After that, we provide the parameters to strace, which then uses them to initiate a call to the kernel's sys trace function and set the mask. The necessary output may now be printed using this syscall().


### Sigalarm and Sigreturn:

To test the working of sigalarm and sigreturn, we have used a program called 'alarmtest.c' and is located in the user directory.

#### Input Command:
```jsx
alarmtest
```

#### Changes:

**kernel/sysproc.c:**

Here we define two system calls: sigalarm and sigreturn, which are necessary for the periodic alerting a process as it uses CPU time. The sigalarm system call is used to set the time interval for the periodic alerting. The sigreturn system call is used to return from the signal handler. In the sigalarm function, we have two variables, ticks and addr, for which we use argint() to collect the user's command line parameters. We then check for command success and handle errors appropriately. We now set the process's alarmticks to the value of the ticks variable, which was passed as a parameter. We also set the process's alarmhandler to the value of the addr variable, which was passed as a parameter. Accordingly, the ticks and address for the process variable is set.

In the sigreturn function, we reset the variables that might have been changed in the handler. We then check for command success and handle errors appropriately. We now set the process's trapframe to the value of the addr variable, which was passed as a parameter. Accordingly, the trapframe for the process variable is set.

**kernel/syscall.c:**

We add the two syscalls to our previously defined arrays, along with the number of arguments that each syscall accepts.

**kernel/trap.c:**

Here we edit both usertrap and kernel trap to add our function, in which we store the trapframe in a local variable trapframe. This local variable is then used to set the alarm trapframe of the process variable, used throughout the function. 

We also set the alarm_on variable, indicating that the handler has been called and used properly. We also keep a track of the ticks of CPU time that the program consumes. When a process calls the alarm function, it checks if the current ticks are greater than the previously initialised ticks. If the former is greater, the trapframe of the process variable sets itself accordingly.

**user/alarmtest.c:**

Here we test out the handler function and check to see if there are race conditions. There are 4 test cases and on passing the test, "Test Passed" is printed. If the test fails, "Test Failed" is printed, with the corresponding error.


## Scheduling Tasks

**Necessary changes and modifications added:**

**Makeﬁle:**

supports SCHEDULER - a macro for the compilation of the speciﬁed scheduling algorithm.

```jsx
#default ROUNDROBIN Scheduler
ifndef SCHEDULER
SCHEDULER:=ROUNDROBIN
endif

#Scheduler Options
CFLAGS += -D $(SCHEDULER)
```

**kernel/proc.h:**

Here we modify `struct proc` to store extra information about the process:

```jsx
  uint64 timecreated;
  uint64 staticpriority;
  uint64 dynamicpriority;
  uint64 numberscheduled;
  uint64 starttime;
  uint64 runtime;
  uint64 sleeptime;
  uint64 stoptime;
  uint64 totruntime;
  uint64 niceness;
  uint64 numberstopped;
```

**kernel/proc.c:**

we intialise all this in `allocproc()` with the respective initial values:

```jsx
  p->cur_ticks = 0;
  p->timecreated = ticks;
  p->staticpriority = 60;
  p->dynamicpriority = 105;
  p->numberscheduled = 0;
  p->starttime = 0;
  p->runtime = 0;
  p->sleeptime = 0;
  p->stoptime = 0;
  p->totruntime = 0;
  p->numberstopped = 0;
```

### 1. FCFS Scheduling

#### Input Command:
```jsx
make qemu SCHEDULER="FCFS"
```

**kernel/proc.c:**

In `scheduler` we do the following:

- So we iterate through all available processes and if this process is runnable then check if there was a process before.
- If no processes were scheduled before then make this the process that is gonna be executed.
- If there are process check if the process's time of creation `p->create_time` is lesser than the current `shortest_time` .
- We then check if we have a process with the minimum time, and then run this process using `swtch(&c->context, &min->context);` .


### 2. Lottery Based Scheduling

#### Input Command:
```jsx
make qemu SCHEDULER="LBS"
```

**kernel/proc.c:**

In `scheduler` we do the following:
- Have a variable total_tickets, which counts the total number of tickets by iterating through all the active processes and adding their tickets.
- Generate a random number between 1 and total_tickets, which we call as the winner.
- Iterate through all the active processes and check when the current number of tickets is greater than the winning (random) integer.
- If such a process is found then run this process using `swtch(&p->context, &c->context);.


## 3. Priority Based Scheduler (PBS)

**kernel/proc.c:**

In `scheduler` we do the following:

- Under struct proc in proc.h, define six new variable time_stopped and time_stopped_temp, times_chosen , static_priority , dynamic_priority and niceness. Also, disable preemption as discussed above.
- Whenever the process goes to sleep, it enters the sleep() function in proc.c. Over here, set time_stopped_temp = ticks. Now go to wakeup(). From here, we can find the time for which the process was sleeping. So, just do time_stopped += (ticks - time_stopped_temp). 
- The waitx() function already checks for the run-time under the variable rtime, so we need not implement is separately. Using p->rtime and p->time_stopped we can calculate niceness.
Now just scan the process table, and schedule the appropriate process according to the priority order <dynamic priority, number of times it was scheduled before, start time>. 
- We can keep updating dynamic_priority , niceness and times_chosen here itself.
- The system call, setpriority() can be easily implemented by making a stub in user space, and changing the appropriate files as we did in spec 1.

### Performance Analysis:

#### RR:
Average RTime: 3
Average WTime: 103

#### FCFS:
Average RTime: 4
Average WTime: 104

#### PBS:
Average RTime: 4
Average WTime: 105

#### LBS:
Average RTime: 5
Average WTime: 107

**Conclusions:**
As we see, the Round Robin Scheduler is the most efficient and the least efficient is the Lottery Based Scheduler. This is because the Round Robin Scheduler is the most fair scheduler, as it gives equal time to all the processes. The Lottery Based Scheduler is the least fair scheduler, as it gives more time to the processes with more tickets. The PBS is also a fair scheduler, but it is not as fair as the RR, as it gives more time to the processes with higher priority. The FCFS is also a fair scheduler, but it is not as fair as the RR, as it gives more time to the processes that were created earlier.

## Copy on Write Fork

- Whenever we create a new child using fork(), we make the child point to the physical address of the memory instead of making a separate copy of the memory. This is done to save memory and time. But, if the child process tries to write to the memory, then we make a copy of the memory and make the child point to the new copy. This is called copy on write fork.

- Each entry of the pagetable has a count variable attached to it to keep track of how many processes are sharing it at any given point of time. As soon as count falls to 0, the function kfree() is used to free the pagetable entries. 
