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

typedef enum {
    ALGO_FCFS,
    ALGO_SJF,
    ALGO_STCF,
    ALGO_RR
} SchedulingAlgorithm;

typedef struct {
    int level;              // Queue priority level (0 = highest)
    int time_quantum;       // Time slice for this queue (-1 for FCFS)
    int allotment;          // Max time before demotion (-1 for infinite)
    Process *queue;         // Array or linked list of processes
    int size;               // Current queue size
} MLFQQueue;

typedef struct {
    MLFQQueue *queues;      // Array of queues
    int num_queues;         // Number of priority levels
    int boost_period;       // Period for priority boost (S)
    int last_boost;         // Last boost time
} MLFQScheduler;

typedef struct {
    Process      *processes;     // Array of all processes
    int           num_processes; // Number of processes
    int           current_time;  // Current simulation time
    GanttChart    gantt;         // Gantt chart for visualization
    int           context_switches;
    int           quantum;       // Time quantum for RR
} SchedulerState;

// Scheduling algorithms
int schedule_fcfs(SchedulerState *state);
int schedule_sjf (SchedulerState *state);
int schedule_stcf(SchedulerState *state);
int schedule_rr  (SchedulerState *state, int quantum);

// Simulation engine
void simulate_scheduler(SchedulerState *state, SchedulingAlgorithm algorithm);

// Utilities
Process *parse_processes_from_string(const char *s, int *out_count);

#endif