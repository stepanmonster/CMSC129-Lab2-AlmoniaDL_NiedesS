#ifndef PROCESS_H
#define PROCESS_H

typedef struct {
    char pid[16];           /* Process identifier                          */
    int arrival_time;       /* When process arrives                        */
    int burst_time;         /* Total CPU time needed                       */
    int remaining_time;     /* For preemptive algorithms                   */
    int start_time;         /* When first executed (for RT)                */
    int finish_time;        /* When completed (for TT)                     */
    int waiting_time;       /* Time spent waiting                          */
    int priority;           /* For MLFQ                                    */
    int time_in_queue;      /* For MLFQ allotment tracking                 */
    int turnaround_time;
    int response_time;
    int last_scheduled_time;
} Process;

/*
 * Parse a single "PID ArrivalTime BurstTime" whitespace-delimited line.
 * Returns a zeroed Process on parse failure.
 */
Process parseProcess(char *input);

/*
 * Load processes from a file (one "PID AT BT" entry per line).
 * On success sets *out_count and returns a heap-allocated array.
 * Returns NULL on error.
 */
Process *loadProcesses(const char *filename, int *out_count);

/*
 * Parse an inline process string in the format documented in README:
 *   "P1,0,5 P2,2,3 P3,4,1"
 * Each token is <PID>,<ArrivalTime>,<BurstTime>; tokens are space-separated.
 * On success sets *out_count and returns a heap-allocated array.
 * Returns NULL on error.
 */
Process *parseProcessString(char *input, int *out_count);

#endif /* PROCESS_H */