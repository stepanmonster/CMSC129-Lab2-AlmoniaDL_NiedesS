#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../include/process.h"
#include "../include/scheduler.h"
#include "../include/metrics.h"
#include "../include/gantt.h"

#define FLAG_INPUT      "--input="
#define FLAG_ALGO       "--algorithm="
#define FLAG_QUANT      "--quantum="
#define FLAG_PROCESSES  "--processes="

// ── Event queue helpers ───────────────────────────────────────────────
static Event *make_event(int time, EventType type, Process *process) {
    Event *e = malloc(sizeof(Event));
    e->time    = time;
    e->type    = type;
    e->process = process;
    e->next    = NULL;
    return e;
}

static void push_event(Event **head, Event *e) {
    if (!*head || e->time < (*head)->time) {
        e->next = *head;
        *head   = e;
        return;
    }
    Event *cur = *head;
    while (cur->next && cur->next->time <= e->time)
        cur = cur->next;
    e->next   = cur->next;
    cur->next = e;
}

static Event *pop_event(Event **head) {
    if (!*head) return NULL;
    Event *e = *head;
    *head    = e->next;
    e->next  = NULL;
    return e;
}

static Event *initialize_events(SchedulerState *state) {
    Event *queue = NULL;
    for (int i = 0; i < state->num_processes; i++)
        push_event(&queue,
                   make_event(state->processes[i].arrival_time,
                               EVENT_ARRIVAL,
                               &state->processes[i]));
    return queue;
}

// ── Simulation engine ─────────────────────────────────────────────────
void simulate_scheduler(SchedulerState *state, SchedulingAlgorithm algorithm) {
    Event *event_queue = initialize_events(state);

    while (event_queue != NULL) {
        Event *current = pop_event(&event_queue);
        state->current_time = current->time;

        switch (current->type) {
            case EVENT_ARRIVAL:        break;
            case EVENT_COMPLETION:     break;
            case EVENT_QUANTUM_EXPIRE: break;
            case EVENT_PRIORITY_BOOST: break;
        }

        free(current);
    }

    switch (algorithm) {
        case ALGO_FCFS:  schedule_fcfs(state);               break;
        case ALGO_SJF:   schedule_sjf(state);                break;
        case ALGO_STCF:  schedule_stcf(state);               break;
        case ALGO_RR:    schedule_rr(state, state->quantum); break;
    }
}

// ── Init helper ───────────────────────────────────────────────────────
static void init_state(SchedulerState *s, Process *procs, int n) {
    memset(s, 0, sizeof(SchedulerState));
    s->processes     = procs;
    s->num_processes = n;
    s->current_time  = 0;
    gantt_init(&s->gantt);
}


// ── Main ──────────────────────────────────────────────────────────────
int main(int argc, char *argv[]) {
    char *input_file  = NULL;
    char *algo_name   = NULL;
    char *proc_string = NULL;
    int   quantum     = 30;

    for (int i = 1; i < argc; i++) {
        if      (strncmp(argv[i], FLAG_INPUT,     strlen(FLAG_INPUT))     == 0)
            input_file  = argv[i] + strlen(FLAG_INPUT);
        else if (strncmp(argv[i], FLAG_ALGO,      strlen(FLAG_ALGO))      == 0)
            algo_name   = argv[i] + strlen(FLAG_ALGO);
        else if (strncmp(argv[i], FLAG_QUANT,     strlen(FLAG_QUANT))     == 0)
            quantum     = atoi(argv[i] + strlen(FLAG_QUANT));
        else if (strncmp(argv[i], FLAG_PROCESSES, strlen(FLAG_PROCESSES)) == 0)
            proc_string = argv[i] + strlen(FLAG_PROCESSES);
    }

    if (!input_file && !proc_string) {
        fprintf(stderr, "Usage: %s --algorithm=<ALGO> --input=<file> [--quantum=<n>]\n", argv[0]);
        fprintf(stderr, "       %s --algorithm=<ALGO> --processes=\"A:0:240,B:10:180\"\n", argv[0]);
        fprintf(stderr, "       %s --compare --input=<file>\n", argv[0]);
        fprintf(stderr, "Algorithms: FCFS SJF STCF RR\n");
        return 1;
    }

    int n = 0;
    Process *procs = input_file
                     ? loadProcesses(input_file, &n)
                     : parse_processes_from_string(proc_string, &n);

    if (!procs || n == 0) {
        fprintf(stderr, "Error: no processes loaded.\n");
        return 1;
    }
    if (!algo_name) {
        fprintf(stderr, "Error: --algorithm= is required.\n");
        free(procs);
        return 1;
    }

    SchedulingAlgorithm algo;
    if      (strcmp(algo_name, "FCFS") == 0) algo = ALGO_FCFS;
    else if (strcmp(algo_name, "SJF")  == 0) algo = ALGO_SJF;
    else if (strcmp(algo_name, "STCF") == 0) algo = ALGO_STCF;
    else if (strcmp(algo_name, "RR")   == 0) algo = ALGO_RR;
    else {
        fprintf(stderr, "Unknown algorithm: %s\n", algo_name);
        free(procs);
        return 1;
    }

    SchedulerState state;
    init_state(&state, procs, n);
    state.quantum = quantum;

    simulate_scheduler(&state, algo);

    gantt_free(&state.gantt);
    free(procs);
    return 0;
}