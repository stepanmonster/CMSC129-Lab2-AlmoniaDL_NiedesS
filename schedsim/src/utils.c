#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../include/scheduler.h"
#include "../include/process.h"
#include "../include/metrics.h"
#include "../include/gantt.h"

SchedulingAlgorithm get_algorithm_type(char *algo_name) {
    if (strcmp(algo_name, "FCFS") == 0) return ALGO_FCFS;
    if (strcmp(algo_name, "SJF")  == 0) return ALGO_SJF;
    if (strcmp(algo_name, "STCF") == 0) return ALGO_STCF;
    if (strcmp(algo_name, "RR")   == 0) return ALGO_RR;
    if (strcmp(algo_name, "MLFQ") == 0) return ALGO_MLFQ;
    fprintf(stderr, "Unknown algorithm: %s\n", algo_name);
    return -1;
}

// Build initial event queue (one ARRIVAL per process)
Event *initialize_events(SchedulerState *state) {
    Event *head = NULL;

    for (int i = 0; i < state->num_processes; i++) {
        // Reset runtime fields so the simulator starts clean
        state->processes[i].remaining_time = state->processes[i].burst_time;
        state->processes[i].start_time     = -1;
        state->processes[i].finish_time    = -1;
        state->processes[i].waiting_time   = 0;
        state->processes[i].turnaround_time = 0;
        state->processes[i].response_time  = 0;

        // add_event keeps the list sorted by time
        add_event(&head, EVENT_ARRIVAL,
                  state->processes[i].arrival_time,
                  &state->processes[i]);
    }

    // If using MLFQ, seed recurring priority-boost events
    if (state->mlfq_config.boost_period > 0) {
        // Find latest arrival to bound the boost events
        int max_burst = 0;
        for (int i = 0; i < state->num_processes; i++)
            max_burst += state->processes[i].burst_time;

        for (int t = state->mlfq_config.boost_period; t < max_burst; t += state->mlfq_config.boost_period)
            add_event(&head, EVENT_PRIORITY_BOOST, t, NULL);
    }

    return head;
}

// Pop front event
Event *pop_event(Event **event_queue) {
    if (event_queue == NULL || *event_queue == NULL) return NULL;
    Event *e = *event_queue;
    *event_queue = e->next;
    e->next = NULL;
    return e;
}

// Move every process back to the top MLFQ queue 
void boost_all_priorities(SchedulerState *state) {
    if (state->mlfq.queues == NULL) return;
    
    mlfq_priority_boost(&state->mlfq, state->current_time);
}

// Print final results (Gantt + metrics table + summary)
void print_results(SchedulerState *state) {
    printf("\n=== Gantt Chart ===\n");
    gantt_print(state->gantt);

    printf("\n=== Per-Process Metrics ===\n");
    print_metrics_table(state->processes, state->num_processes);

    // Build summary
    SummaryMetrics *s = &state->summary_metrics;
    s->avg_turnaround  = avg_turnaround_time(state->processes, state->num_processes);
    s->avg_waiting     = avg_waiting_time(state->processes, state->num_processes);
    s->avg_response    = avg_response_time(state->processes, state->num_processes);
    s->context_switches = state->context_switches;

    int total_time = state->current_time;
    double util    = total_time > 0 ? (100.0 * state->cpu_busy / total_time) : 0.0;

    printf("\n=== Summary ===\n");
    printf("Total time       : %d\n",    total_time);
    printf("CPU utilisation  : %.1f%%\n", util);
    printf("Context switches : %d\n",    s->context_switches);
    print_summary(s);
}