#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../include/scheduler.h"
#include "../include/metrics.h"
#include "../include/gantt.h"

// ─── Find shortest job in ready queue ────────────────────────────────
static int find_shortest_job(Process *procs, int n, int current_time) {
    int shortest = -1;
    int min_burst = __INT_MAX__;

    for (int i = 0; i < n; i++) {
        // must have arrived and not yet finished
        if (procs[i].arrival_time <= current_time &&
            procs[i].finish_time == -1) {
            if (procs[i].burst_time < min_burst) {
                min_burst = procs[i].burst_time;
                shortest  = i;
            }
            // tie-break: alphabetical by PID
            else if (procs[i].burst_time == min_burst) {
                if (strcmp(procs[i].pid, procs[shortest].pid) < 0)
                    shortest = i;
            }
        }
    }
    return shortest;  // returns index, -1 if none ready
}

// ─── Check if all processes are done ─────────────────────────────────
static int all_complete(Process *procs, int n) {
    for (int i = 0; i < n; i++)
        if (procs[i].finish_time == -1) return 0;
    return 1;
}

// ─── SJF Scheduler ───────────────────────────────────────────────────
int schedule_sjf(SchedulerState *state) {
    printf("Running SJF Scheduler...\n\n");

    Process *procs = state->processes;
    int       n    = state->num_processes;

    // reset state
    for (int i = 0; i < n; i++) {
        procs[i].remaining_time = procs[i].burst_time;
        procs[i].start_time     = -1;
        procs[i].finish_time    = -1;
        procs[i].waiting_time   = 0;
    }

    state->current_time    = 0;
    state->context_switches = 0;

    while (!all_complete(procs, n)) {
        int idx = find_shortest_job(procs, n, state->current_time);

        // no process ready — CPU idle, advance time to next arrival
        if (idx == -1) {
            state->current_time++;
            continue;
        }

        Process *p = &procs[idx];

        // record first execution time (for response time)
        if (p->start_time == -1)
            p->start_time = state->current_time;

        // context switch if different process
        if (idx != -1) state->context_switches++;

        // record gantt entry — runs to completion (non-preemptive)
        int start = state->current_time;
        state->current_time += p->burst_time;
        gantt_add(&state->gantt, p->pid, start, state->current_time);

        // mark complete
        p->finish_time    = state->current_time;
        p->remaining_time = 0;
    }

    // ── print results ──
    printf("=== Gantt Chart ===\n");
    gantt_print(&state->gantt);

    printf("\n=== Metrics ===\n");
    calculate_metrics(procs, n);
    print_metrics_table(procs, n);

    return 0;
}
