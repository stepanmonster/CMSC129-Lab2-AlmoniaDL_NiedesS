#include <stdio.h>
#include "../include/process.h"
#include "../include/scheduler.h"

int schedule_fcfs(SchedulerState *state) {
    if (state->ready_count == 0) return -1;

    // FCFS always takes the first process in the queue
    Process *next = state->ready_queue[0];

    // Remove from queue (shift everything left)
    for (int i = 0; i < state->ready_count - 1; i++) {
        state->ready_queue[i] = state->ready_queue[i + 1];
    }
    state->ready_count--;

    state->current_process = next;
    return 0;
}
