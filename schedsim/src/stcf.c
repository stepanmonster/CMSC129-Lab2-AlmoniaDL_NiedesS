#include <stdio.h>
#include "../include/process.h"
#include "../include/scheduler.h"

int schedule_stcf(SchedulerState *state) {
    if (state->ready_count == 0) return -1;

    int shortest_idx = 0;
    for (int i = 1; i < state->ready_count; i++) {
        // Compare remaining_time, not burst_time
        if (state->ready_queue[i]->remaining_time < state->ready_queue[shortest_idx]->remaining_time) {
            shortest_idx = i;
        }
    }

    Process *next = state->ready_queue[shortest_idx];

    for (int i = shortest_idx; i < state->ready_count - 1; i++) {
        state->ready_queue[i] = state->ready_queue[i + 1];
    }
    state->ready_count--;

    state->current_process = next;
    return 0;
}
