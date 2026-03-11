# MLFQ Design Notes
For our MLFQ design, we opted for 4 priority levels for the 4 different workloads typically observed in an actual operating system environment. These tasks are usually instant response tasks (e.g. GUI interrupts and updates) that have usually have burst times less than 10ms, interactive tasks that require more than a simple interrupt (e.g. shell processing a command) that require a bit more than 10ms of CPU time, medium-length CPU bursts (e.g. jobs that use the processor for a good amount of time without stopping for I/O with BT around 40ms-200ms), and the CPU-intensive tasks with virtually no I/O wait times (BT > 200ms).

### The 4 levels of the MLFQ will be as follows:
- **Level 0**: The 'Instant Response' Queue. This level of the MLFQ will have the lowest time quantum of 10ms. This queue is designed for I/O-bound tasks, which use the CPU very minimally and finish even before the quantum has expired. Allotment for this level will be equal to the quantum.
- **Level 1**: The Interactive-Heavy Queue. This is for processes that exceed the quantum of the first level. The tasks found here are usually those that do not need a lot of CPU time but require more than a simple interrupt. This level will have a time quantum of 30ms. Similar to the first level, allotment in this level will be equal to time quantum.
- **Level 2**: This level deals with processes that are behaving in a CPU-bound manner, meaning they are beginning to have lesser I/O wait times. This level acts as a buffer that filters out medium-sized tasks before they reach the final batch queue. The time quantum used for this level will be 80 ms. A strictly tracked allotment will be used in this level to avoid a process gaming the system by yielding just before the 80ms limit.
- **Level 3**: The last level will have the biggest time quantum. This is basically the level for the longest-running batch jobs. This is where the most CPU-bound tasks will go since they will have the largest CPU time usage. The time quantum used for this level will be 200ms. A large time quantum will be used to minimize context-switching overhead and focus more on computing.

The MLFQ will also use a boost period of **500ms.**

**Justification:** 
A Multi-Level Feedback Queue is prone to starvation. If the system is constantly flooded with new "Instant Response" tasks (Level 0), the CPU-bound tasks in Level 3 may never get a chance to run. To prevent this, we implement a Priority Boost.

**Mechanism:** 
After a fixed time interval , every process in the system—regardless of its current level or how much of its allotment it has used—is moved back to Level 0.

**Reasoning:** 
1.  Guaranteed Progress: It ensures that even the "heavy" background jobs in Level 3 get at least a few milliseconds of CPU time every second.
2.  Dynamic Adaptation: If a process was CPU-bound for a long time but suddenly becomes interactive again (e.g., a background downloader finishing and waiting for user input), the boost allows the scheduler to "re-learn" the process's behavior and keep it at a higher priority.