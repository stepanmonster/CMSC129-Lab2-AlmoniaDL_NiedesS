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

static int find_shortest_remaining(Process *procs, int n, int current_time) {
    int best = -1;
    for (int i = 0; i < n; i++) {
        if (procs[i].arrival_time <= current_time && procs[i].finish_time == -1) {
            if (best == -1 ||
                procs[i].remaining_time < procs[best].remaining_time ||
                (procs[i].remaining_time == procs[best].remaining_time &&
                 strcmp(procs[i].pid, procs[best].pid) < 0))
                best = i;
        }
    }
    return best;
}

int schedule_stcf(SchedulerState *state) {
    printf("Running STCF Scheduler...\n\n");

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

    int prev_idx = -1;

    while (!all_complete(procs, n)) {
        int idx = find_shortest_remaining(procs, n, state->current_time);

        if (idx == -1) {
            state->current_time++;
            continue;
        }

        Process *p = &procs[idx];
        if (p->start_time == -1) p->start_time = state->current_time;

        if (idx != prev_idx) {
            state->context_switches++;
            if (prev_idx != -1)
                printf("Process %s preempted at t=%d (remaining: %d)\n",
                       procs[prev_idx].pid, state->current_time,
                       procs[prev_idx].remaining_time);
        }

        // Advance to next arrival or completion
        int next_event = state->current_time + p->remaining_time;
        for (int i = 0; i < n; i++)
            if (procs[i].arrival_time > state->current_time &&
                procs[i].arrival_time < next_event &&
                procs[i].finish_time == -1)
                next_event = procs[i].arrival_time;

        int elapsed = next_event - state->current_time;
        gantt_add(&state->gantt, p->pid, state->current_time, next_event);
        p->remaining_time  -= elapsed;
        state->current_time = next_event;

        if (p->remaining_time == 0) {
            p->finish_time = state->current_time;
            printf("Process %s completed at t=%d\n", p->pid, p->finish_time);
            prev_idx = -1;
        } else {
            prev_idx = idx;
        }
    }

    printf("\n=== Gantt Chart ===\n");
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

    return 0;
}