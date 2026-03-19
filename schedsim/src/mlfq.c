#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../include/scheduler.h"
#include "../include/process.h"

void mlfq_queue_init(MLFQQueue *q, int level, int allotment, int time_quantum) {
    q->level       = level;
    q->allotment   = allotment;
    q->time_quantum = time_quantum;
    q->size        = 0;
    q->capacity    = 8;
    q->queue       = malloc(sizeof(Process *) * q->capacity);
}

void enqueue(MLFQQueue *q, Process *p) {
    if (q->size >= q->capacity) {
        q->capacity *= 2; // Dynamically create space for new queue entry
        q->queue = realloc(q->queue, sizeof(Process *) * q->capacity);
    }
    q->queue[q->size++] = p;
}

Process *dequeue(MLFQQueue *q) {
    if (q->size == 0) return NULL;
    Process *p = q->queue[0];
    /* Shift left */
    for (int i = 0; i < q->size - 1; i++)
        q->queue[i] = q->queue[i + 1];
    q->size--;
    return p;
}

void remove_from_queue(MLFQQueue *q, Process *p) {
    for (int i = 0; i < q->size; i++) {
        if (q->queue[i] == p) {
            for (int j = i; j < q->size - 1; j++)
                q->queue[j] = q->queue[j + 1];
            q->size--;
            return;
        }
    }
}

void add_to_queue(MLFQQueue *q, Process *p) {
    enqueue(q, p);
}

/* ── Priority adjustment (call after a process uses its quantum) ── */
void mlfq_adjust_priority(MLFQScheduler *scheduler, Process *p) {
    if (p->priority >= scheduler->num_queues - 1) return; /* already lowest */

    MLFQQueue *current_queue = &scheduler->queues[p->priority];

    if (p->time_in_queue >= current_queue->allotment) {
        remove_from_queue(current_queue, p);
        p->priority++;
        p->time_in_queue = 0;
        add_to_queue(&scheduler->queues[p->priority], p);
    }
}

/* ── Priority boost: all processes → queue 0 ─────────────────────── */
void mlfq_priority_boost(MLFQScheduler *scheduler, int current_time) {
    if (scheduler == NULL || scheduler->queues == NULL) return;
    
    if (current_time - scheduler->last_boost >= scheduler->boost_period) {
        for (int i = 1; i < scheduler->num_queues; i++) {
            MLFQQueue *queue = &scheduler->queues[i];
            while (queue->size > 0) {
                Process *p = dequeue(queue);
                p->priority = 0;
                p->time_in_queue = 0;
                enqueue(&scheduler->queues[0], p);
            }
        }
        scheduler->last_boost = current_time;
    }
}
/* ── Scheduler: pick the highest-priority non-empty queue ─────────── */
int schedule_mlfq(SchedulerState *state, MLFQConfig *config) {
    MLFQScheduler *scheduler = &state->mlfq;

    for (int i = 0; i < scheduler->num_queues; i++) {
        MLFQQueue *q = &scheduler->queues[i];

        if (q->size > 0) {
            Process *next = dequeue(q);
            state->current_process = next;
            /* Set the quantum for this priority level */
            state->quantum = config->time_quantums[i];
            return 0;
        }
    }
    return -1; /* Nothing ready */
}


/* ── Default 4-level MLFQ config ──────────────────────────────────── */
MLFQConfig createConfig(void) {
    MLFQConfig config;
    config.num_queues   = 4;
    config.boost_period = 500;

    config.time_quantums = malloc(sizeof(int) * 4);
    config.allotments    = malloc(sizeof(int) * 4);

    config.time_quantums[0] = 10;
    config.time_quantums[1] = 30;
    config.time_quantums[2] = 80;
    config.time_quantums[3] = 200;

    config.allotments[0] = 10;
    config.allotments[1] = 30;
    config.allotments[2] = 160;
    config.allotments[3] = -1;   /* bottom queue: no demotion */

    return config;
}