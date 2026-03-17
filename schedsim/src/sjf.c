#include <stdio.h>
#include <string.h>
#include "../include/process.h"
#include "../include/scheduler.h"

/*
 * Non-preemptive SJF: pick the ready process with the shortest burst_time.
 * Tie-break: alphabetical by PID.
 *
 * The engine in main.c is non-preemptive for SJF (no STCF-style re-queuing
 * on arrival), so once we return a process it runs to completion before the
 * engine calls us again.
 */
int schedule_sjf(SchedulerState *state) {
    if (state->ready_count == 0) return -1;

    int shortest_idx = 0;
    for (int i = 1; i < state->ready_count; i++) {
        Process *candidate = state->ready_queue[i];
        Process *current   = state->ready_queue[shortest_idx];

        if (candidate->burst_time < current->burst_time) {
            shortest_idx = i;
        } else if (candidate->burst_time == current->burst_time) {
            // Tie-break: alphabetical PID
            if (strcmp(candidate->pid, current->pid) < 0)
                shortest_idx = i;
        }
    }

    Process *next = state->ready_queue[shortest_idx];

    // Remove from ready queue
    for (int i = shortest_idx; i < state->ready_count - 1; i++)
        state->ready_queue[i] = state->ready_queue[i + 1];
    state->ready_count--;

    state->current_process = next;
    return 0;
}