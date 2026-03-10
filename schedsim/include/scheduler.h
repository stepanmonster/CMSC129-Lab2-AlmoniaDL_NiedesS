#ifndef SCHEDULER_H
#define SCHEDULER_H

#include "process.h"
#include "gantt.h"

typedef enum {
    EVENT_ARRIVAL,
    EVENT_COMPLETION,
    EVENT_QUANTUM_EXPIRE,
    EVENT_PRIORITY_BOOST
} EventType;

typedef struct Event {
    int time;
    EventType type;
    Process *process;
    struct Event *next;
} Event;

typedef enum{
    ALGO_FCFS,
    ALGO_SJF,
    ALGO_STCF,
    ALGO_RR,
    ALGO_MLFQ
}SchedulingAlgorithm;

typedef struct {
    int num_queues;         // Number of priority levels
    int *time_quantums;     // Time quantum for each level (-1 for FCFS)
    int *allotments;        // Allotment for each level (-1 for infinite)
    int boost_period;       // Period for priority boost (S)
} MLFQConfig;

typedef struct {
    Process *processes;     // Array of all processes
    int num_processes;      // Number of processes
    int current_time;       // Current simulation time
    GanttChart gantt;       // Gantt chart for visualization
    int context_switches;   // Count of context switches
    MLFQScheduler mlfq;       // MLFQ state (if using MLFQ)
    // ... additional fields for metrics, Gantt chart, etc.
    // Recall: CMSC 141
} SchedulerState;

// Return 0 on success, -1 on error (command line etiquette)
int schedule_fcfs(SchedulerState *state);
int schedule_sjf(SchedulerState *state);
int schedule_stcf(SchedulerState *state);
int schedule_rr(SchedulerState *state, int quantum);
int schedule_mlfq(SchedulerState *state, MLFQConfig *config);

#endif