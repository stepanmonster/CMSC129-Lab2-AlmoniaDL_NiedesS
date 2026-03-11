#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../include/scheduler.h"
#include "../include/metrics.h"
#include "../include/gantt.h"

// RR uses MLFQQueue (from scheduler.h) as its single ready queue
static void rq_enqueue(MLFQQueue *q, Process *p) {
    q->queue = realloc(q->queue, (q->size + 1) * sizeof(Process));
    q->queue[q->size++] = *p;
}

static Process rq_dequeue(MLFQQueue *q) {
    Process p = q->queue[0];
    for (int i = 1; i < q->size; i++) q->queue[i-1] = q->queue[i];
    q->size--;
    return p;
}

int schedule_rr(SchedulerState *state, int quantum) {
    printf("Running RR Scheduler...\n");
    printf("Using time quantum q=%d\n\n", quantum);

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

    MLFQQueue ready;
    ready.level        = 0;
    ready.time_quantum = quantum;
    ready.allotment    = -1;
    ready.size         = 0;
    ready.queue        = malloc(n * sizeof(Process));

    int *in_queue = calloc(n, sizeof(int));

    for (int i = 0; i < n; i++)
        if (procs[i].arrival_time == 0) {
            rq_enqueue(&ready, &procs[i]);
            in_queue[i] = 1;
        }

    int completed = 0;

    while (completed < n) {
        if (ready.size == 0) {
            int next = INT_MAX;
            for (int i = 0; i < n; i++)
                if (procs[i].finish_time == -1 && procs[i].arrival_time > state->current_time)
                    if (procs[i].arrival_time < next) next = procs[i].arrival_time;
            state->current_time = next;
            for (int i = 0; i < n; i++)
                if (!in_queue[i] && procs[i].finish_time == -1 &&
                    procs[i].arrival_time <= state->current_time) {
                    rq_enqueue(&ready, &procs[i]);
                    in_queue[i] = 1;
                }
            continue;
        }

        Process front = rq_dequeue(&ready);
        int idx = -1;
        for (int i = 0; i < n; i++)
            if (strcmp(procs[i].pid, front.pid) == 0) { idx = i; break; }

        Process *p = &procs[idx];
        if (p->start_time == -1) p->start_time = state->current_time;

        state->context_switches++;

        int run = (p->remaining_time < quantum) ? p->remaining_time : quantum;
        gantt_add(&state->gantt, p->pid, state->current_time, state->current_time + run);
        p->remaining_time  -= run;
        state->current_time += run;

        // Enqueue newly arrived processes before re-adding current
        for (int i = 0; i < n; i++)
            if (!in_queue[i] && procs[i].finish_time == -1 &&
                procs[i].arrival_time <= state->current_time) {
                rq_enqueue(&ready, &procs[i]);
                in_queue[i] = 1;
            }

        if (p->remaining_time == 0) {
            p->finish_time = state->current_time;
            completed++;
        } else {
            rq_enqueue(&ready, p);
        }
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

    return 0;
}