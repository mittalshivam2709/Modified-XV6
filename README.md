The directory structure is same as required for the project.
The report for partB of networks is present in the partB directory.

The analysis of rtimes and wtimes for different scheduling policies is as follows:-

roundrobin : rtime = 18 wtime = 177
fcfs       : rtime = 24 wtime = 144
mlfq       : rtime = 19 wtime = 123       
In roundrobin the process are executed in a cyclic order one by one and the cpu performs a context switch after each tick whereas in fcfs the cpu completes executing one process and then goes to the other, this is why the response time is more in fcfs. For mlfq  we can say that every time a new process comes it is added to the end of the first queue in which the time slice is one tick. hence the first time when the process gets the cpu control is similar to round robin hence the response time is similar to that of round robin.

the graph my_plot.png shows that the cpu bound processes are in queues 1,2,3 and 4 and their priority is being reduced timely. The file test.txt contains the data which has bee used for creating the graph.
I have not handled aging however it can be done by using a for loop at the beginning of the scheduler function. 