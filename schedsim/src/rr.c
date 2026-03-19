#include <stdio.h>
#include "../include/process.h"
#include "../include/scheduler.h"

/*NOTE:
 * Round-Robin simply takes the front of the ready queue.
 * The time-slice enforcement (quantum expiry) is handled by the
 * event-driven engine in main.c — schedule_next_event() will emit
 * EVENT_QUANTUM_EXPIRE when remaining_time > quantum, and
 * handle_quantum_expire() re-enqueues the process at the back.
 */
int schedule_rr(SchedulerState *state, int quantum) {
    (void)quantum; // quantum is managed by the DES engine, not here

    if (state->ready_count == 0) return -1;

    // Take the front of the queue (FIFO order within the queue) 
    Process *next = state->ready_queue[0];

    // Shift queue left
    for (int i = 0; i < state->ready_count - 1; i++)
        state->ready_queue[i] = state->ready_queue[i + 1];
    state->ready_count--;

    state->current_process = next;
    return 0;
}