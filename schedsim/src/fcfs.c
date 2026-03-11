#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../include/scheduler.h"
#include "../include/metrics.h"
#include "../include/gantt.h"

static int all_complete(Process *procs, int n) {
    for (int i = 0; i < n; i++)
        if (procs[i].finish_time == -1) return 0;
    return 1;
}

static int find_next_fcfs(Process *procs, int n, int current_time) {
    int best = -1;
    for (int i = 0; i < n; i++) {
        if (procs[i].arrival_time <= current_time && procs[i].finish_time == -1) {
            if (best == -1 ||
                procs[i].arrival_time < procs[best].arrival_time ||
                (procs[i].arrival_time == procs[best].arrival_time &&
                 strcmp(procs[i].pid, procs[best].pid) < 0))
                best = i;
        }
    }
    return best;
}

int schedule_fcfs(SchedulerState *state) {
    printf("Running FCFS Scheduler...\n\n");

    Process *procs = state->processes;
    int n = state->num_processes;

    for (int i = 0; i < n; i++) {
        procs[i].remaining_time = procs[i].burst_time;
        procs[i].start_time     = -1;
        procs[i].finish_time    = -1;
        procs[i].waiting_time   = 0;
    }

    state->current_time     = 0;
    state->context_switches = 0;

    while (!all_complete(procs, n)) {
        int idx = find_next_fcfs(procs, n, state->current_time);

        if (idx == -1) {
            state->current_time++;
            continue;
        }

        Process *p = &procs[idx];
        if (p->start_time == -1) p->start_time = state->current_time;

        state->context_switches++;

        int start = state->current_time;
        state->current_time += p->burst_time;
        gantt_add(&state->gantt, p->pid, start, state->current_time);

        p->finish_time    = state->current_time;
        p->remaining_time = 0;
    }

    printf("=== Gantt Chart ===\n");
    gantt_print(&state->gantt);

    printf("\n=== Metrics ===\n");
    calculate_metrics(procs, n);
    print_metrics_table(procs, n);

    SummaryMetrics summary;
    summary.avg_turnaround   = avg_turnaround_time(procs, n);
    summary.avg_waiting      = avg_waiting_time(procs, n);
    summary.avg_response     = avg_response_time(procs, n);
    summary.context_switches = state->context_switches;
    print_summary(&summary);

    for (int i = 0; i < n; i++)
        if (procs[i].waiting_time > 0) {
            printf("\nConvoy effect detected: Process %s waited %d time units\n",
                   procs[i].pid, procs[i].waiting_time);
            break;
        }

    return 0;
}