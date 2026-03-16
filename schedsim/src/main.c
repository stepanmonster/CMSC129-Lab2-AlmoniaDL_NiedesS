#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../include/scheduler.h"
#include "../include/process.h"

#define FLAG_INPUT "--input="
#define FLAG_ALGO  "--algorithm="
#define FLAG_QUANT "--quantum="

void handle_arrival(SchedulerState *state, Process *process){
    if (state->ready_count == state->ready_capacity) {
        state->ready_capacity = (state->ready_capacity == 0) ? 4 : state->ready_capacity * 2;
        state->ready_queue = realloc(state->ready_queue, state->ready_capacity * sizeof(Process*));
    }

    state->ready_queue[state->ready_count] = process;
    state->ready_count++;

    process->arrival_time = state->current_time;
}

void handle_completion(SchedulerState *state, Process *process){
    process->finish_time = state->current_time;

    process->turnaround_time = process->finish_time - process->arrival_time;

    state->current_process = NULL;
}


void handle_preemption(SchedulerState *state){
    
}

void handle_boost(SchedulerState *state){
    
}

void add_to_queue(SchedulerState *state, Event event){
    
}

Event* pop_event(Event **event_queue){
    if(event_queue == NULL || *event_queue == NULL ){
        return NULL;
    }

    Event *popped = *event_queue;

    *event_queue = popped->next; // update head of queue

    return popped;
}

void add_event(Event **head, Event *new_event) {
    // Case 1: Queue is empty OR new event happens before the current head
    if (*head == NULL || new_event->time < (*head)->time) {
        new_event->next = *head;
        *head = new_event;
        return;
    }

    // Case 2: Traverse the list to find the correct chronological spot
    Event *current = *head;
    while (current->next != NULL && current->next->time <= new_event->time) {
        current = current->next;
    }

    // Insert the new event into the middle or at the end
    new_event->next = current->next;
    current->next = new_event;
}

// NOTE: bali store danay tanan sa scheduler state (probably main loop) and then pass to simulate_scheduler
void simulate_scheduler(SchedulerState *state, SchedulingAlgorithm algorithm) {
    Event *event_queue = initialize_events(state); // what the hell do i do with this
    
    while (event_queue != NULL) {
        Event *current = pop_event(&event_queue);
        state->current_time = current->time;
        
        switch (current->type) {
            case EVENT_ARRIVAL:
                // Put the process into the ready queue
                handle_arrival(state, current->process); 

                // If the CPU is idle, we should start this process and schedule its completion.
                if (state->current_process == NULL) {
                    state->current_process = current->process; // Simplified logic
                    
                    // Create a BRAND NEW event for the future
                    Event *completion = malloc(sizeof(Event));
                    completion->type = EVENT_COMPLETION;
                    completion->process = current->process;
                    // The event happens in the future: Now + Burst Time
                    completion->time = state->current_time + current->process->burst_time;
                    
                    // Enqueue new event
                    add_event(&event_queue, completion);
                }
                break;
            case EVENT_COMPLETION:
                handle_completion(state, current->process);
                break;
            case EVENT_QUANTUM_EXPIRE:
                handle_preemption(state);
                break;
            case EVENT_PRIORITY_BOOST:
                handle_boost(state);
                break;
        }
        free(current);
    }
    calculate_metrics(state, state->num_processes);
    print_results(state);
}

int main(int argc, char *argv[]) {
    char *input_file = NULL;
    char *algo_name = NULL;
    int quantum = 50; 

    // 1. Parse Arguments
    for (int i = 1; i < argc; i++) {
        if (strncmp(argv[i], FLAG_INPUT, strlen(FLAG_INPUT)) == 0) {
            input_file = argv[i] + strlen(FLAG_INPUT);
        } 
        else if (strncmp(argv[i], FLAG_ALGO, strlen(FLAG_ALGO)) == 0) {
            algo_name = argv[i] + strlen(FLAG_ALGO);
        } 
        else if (strncmp(argv[i], FLAG_QUANT, strlen(FLAG_QUANT)) == 0) {
            quantum = atoi(argv[i] + strlen(FLAG_QUANT));
        }
    }

    if (!input_file || !algo_name) {
        fprintf(stderr, "Usage: %s %s<file> %s<name> [%s<n>]\n", 
                argv[0], FLAG_INPUT, FLAG_ALGO, FLAG_QUANT);
        return 1;
    }

    int count = 0;
    Process *process_list = loadProcesses(input_file, &count);
    if (!process_list) return 1;

    SchedulerState state;
    memset(&state, 0, sizeof(SchedulerState));
    state.processes = process_list;
    state.num_processes = count;
    state.context_switches = 0;
    // state.quantum = quantum; 

    SchedulingAlgorithm selected_algo = get_algorithm_type(algo_name);
    printf("Running %s on %s (Quantum: %d)\n", algo_name, input_file, quantum);
    
    simulate_scheduler(&state, selected_algo);

    free(process_list);
    return 0;
}