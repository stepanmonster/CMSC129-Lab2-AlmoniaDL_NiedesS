#ifndef SCHEDULER_H
#define SCHEDULER_H

#include "process.h"
#include "gantt.h"
#include "metrics.h"

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
    int level;              // Queue priority level (0 = highest)
    int time_quantum;       // Time slice for this queue (-1 for FCFS)
    int allotment;          // Max time before demotion (-1 for infinite)
    Process **queue;         // Array or linked list of processes
    int size;               // Current queue size
    int capacity;
} MLFQQueue;

typedef struct {
    MLFQQueue *queues;      // Array of queues
    int num_queues;         // Number of priority levels
    int boost_period;       // Period for priority boost (S)
    int last_boost;         // Last boost time
} MLFQScheduler;

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
    GanttChart *gantt;       // Gantt chart for visualization
    int context_switches;   // Count of context switches
    MLFQScheduler mlfq;       // MLFQ state (if using MLFQ) 
    int quantum;            // for RR
    SummaryMetrics summary_metrics;
    Process *current_process;
    int cpu_busy;
    Process **ready_queue;
    int ready_count;
    int ready_capacity;
    MLFQConfig mlfq_config;
} SchedulerState;

// Return 0 on success, -1 on error (command line etiquette)
int schedule_fcfs(SchedulerState *state);
int schedule_sjf(SchedulerState *state);
int schedule_stcf(SchedulerState *state);
int schedule_rr(SchedulerState *state, int quantum);
int schedule_mlfq(SchedulerState *state, MLFQConfig *config);

// include/scheduler.h
Event* pop_event(Event **event_queue);
void add_event(Event **head, EventType type, int time, Process *proc);
void schedule_next_event(SchedulerState *state, SchedulingAlgorithm algorithm, Event **event_queue);
SchedulingAlgorithm get_algorithm_type(char *name);
void boost_all_priorities(SchedulerState *state);

void handle_arrival(SchedulerState *state, Process *process);
void handle_completion(SchedulerState *state, Process *process);
void handle_quantum_expire(SchedulerState *state);
void record_progress(SchedulerState *state, int next_time);

MLFQConfig createConfig(void);

/* MLFQ queue helpers */
void     mlfq_queue_init(MLFQQueue *q, int level, int allotment, int time_quantum);
void     enqueue(MLFQQueue *q, Process *p);
Process *dequeue(MLFQQueue *q);
void     remove_from_queue(MLFQQueue *q, Process *p);
void     add_to_queue(MLFQQueue *q, Process *p);
void     mlfq_adjust_priority(MLFQScheduler *scheduler, Process *p);
void     mlfq_priority_boost(MLFQScheduler *scheduler, int current_time);

/* Utils */
Event             *initialize_events(SchedulerState *state);
void               print_results(SchedulerState *state);

#endif