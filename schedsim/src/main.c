#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../include/scheduler.h"
#include "../include/process.h"

void add_event(Event **head, EventType type, int time, Process *proc) {
    Event *e = malloc(sizeof(Event));
    if (!e) {
        fprintf(stderr, "Error: out of memory in add_event\n");
        exit(1);
    }
    e->type    = type;
    e->time    = time;
    e->process = proc;
    e->next    = NULL;

    // Priority: ARRIVAL=0 (processed first), QUANTUM_EXPIRE/COMPLETION=1 (processed second)
    int new_priority = (type == EVENT_ARRIVAL) ? 0 : 1;

    // Insert at head if: list is empty, new event is earlier, OR same time but higher priority
    if (*head == NULL || e->time < (*head)->time ||
        (e->time == (*head)->time &&
         new_priority < ((*head)->type == EVENT_ARRIVAL ? 0 : 1))) {
        e->next = *head;
        *head   = e;
        return;
    }

    Event *cur = *head;
    while (cur->next != NULL) {
        int next_priority = (cur->next->type == EVENT_ARRIVAL) ? 0 : 1;
        // Use strict > so equal-priority events at the same time keep FIFO order
        if (cur->next->time > e->time ||
            (cur->next->time == e->time && next_priority > new_priority))
            break;
        cur = cur->next;
    }

    e->next   = cur->next;
    cur->next = e;
}

void schedule_next_event(SchedulerState *state,
                         SchedulingAlgorithm algorithm,
                         Event **event_queue) {
    if (state->current_process == NULL) {
        fprintf(stderr, "Error: schedule_next_event called with no current process\n");
        return;
    }
    int time_to_run = state->current_process->remaining_time;
    EventType type  = EVENT_COMPLETION;

    if (algorithm == ALGO_RR || algorithm == ALGO_MLFQ) {
        if (time_to_run > state->quantum) {
            time_to_run = state->quantum;
            type        = EVENT_QUANTUM_EXPIRE;
        }
    }
    add_event(event_queue, type,
              state->current_time + time_to_run,
              state->current_process);
}

void handle_arrival(SchedulerState *state, Process *process) {
    if (state->ready_count == state->ready_capacity) {
        int new_capacity = (state->ready_capacity == 0) ? 4 : state->ready_capacity * 2;
        Process **tmp = realloc(state->ready_queue, new_capacity * sizeof(Process *));
        if (!tmp) {
            fprintf(stderr, "Error: out of memory in handle_arrival\n");
            exit(1);
        }
        state->ready_queue    = tmp;
        state->ready_capacity = new_capacity;
    }
    state->ready_queue[state->ready_count++] = process;
}

void handle_completion(SchedulerState *state, Process *process) {
    process->finish_time   = state->current_time;
    state->current_process = NULL;
}

void handle_quantum_expire(SchedulerState *state) {
    if (state->current_process == NULL) return;

    if (state->mlfq.queues != NULL) {
        // last_scheduled_time is -1 if the process never ran (shouldn't normally
        // happen here, but guard to avoid computing a nonsense elapsed value).
        int elapsed = (state->current_process->last_scheduled_time >= 0)
                      ? state->current_time - state->current_process->last_scheduled_time
                      : 0;
        state->current_process->time_in_queue += elapsed;
        mlfq_adjust_priority(&state->mlfq, state->current_process);
        enqueue(&state->mlfq.queues[state->current_process->priority],
                state->current_process);
    } else {
        handle_arrival(state, state->current_process);
    }

    {
    int has_other = 0;
    if (state->mlfq.queues != NULL) {
        for (int _qi = 0; _qi < state->mlfq.num_queues && !has_other; _qi++)
            has_other = (state->mlfq.queues[_qi].size > 0);
    } else {
        has_other = (state->ready_count > 0);
    }
    if (has_other) state->context_switches++;
}
state->current_process = NULL;
}

// Handle RR quantum expiration as a single, self-contained operation.
//
// The original inline code in simulate_scheduler was fragile because it:
//   1. Read current_process->last_scheduled_time and computed insert_pos,
//   2. Then called handle_quantum_expire(), which NULLs current_process and
//      appends the process to the back of ready_queue as a side effect,
//   3. Then shifted the process from the back to insert_pos.
//
// All three steps are now combined here so the ordering and side-effects are
// explicit and cannot be accidentally reordered by a future edit.
//
// RR semantics: an expired process should be placed after all processes that
// were already in the ready queue when its quantum started (i.e., after any
// process whose arrival_time <= quantum_start), but before any that arrived
// during its quantum.
void handle_rr_quantum_expire(SchedulerState *state) {
    if (state->current_process == NULL) return;

    // Step 1: capture identity of the expiring process and when its quantum
    // started, before handle_quantum_expire() clears current_process.
    Process *expiring    = state->current_process;
    int      quantum_start = expiring->last_scheduled_time;

    // Step 2: find the insertion position — after every process that was
    // already waiting when this quantum began.
    int insert_pos = 0;
    for (int i = 0; i < state->ready_count; i++) {
        if (state->ready_queue[i]->arrival_time <= quantum_start)
            insert_pos = i + 1;
    }

    // Step 3: re-enqueue the expiring process at the back (handle_quantum_expire
    // calls handle_arrival internally) and NULL out current_process.
    handle_quantum_expire(state);   // appends expiring to ready_queue[ready_count-1]
                                    // and sets current_process = NULL

    // Step 4: slide the process from the back to insert_pos.
    for (int i = state->ready_count - 1; i > insert_pos; i--)
        state->ready_queue[i] = state->ready_queue[i - 1];
    state->ready_queue[insert_pos] = expiring;
}

void record_progress(SchedulerState *state, int next_time) {
    int duration = next_time - state->current_time;
    if (duration > 0 && state->current_process != NULL) {
        state->current_process->remaining_time -= duration;
        state->cpu_busy += duration;
        gantt_add(state->gantt,
                  state->current_process->pid,
                  state->current_time,
                  next_time);
    }
}

// ---------------------------------------------------------------------------
// Core event loop — shared by both the verbose and silent simulation paths.
//
// All scheduling logic lives here. The caller decides what to do with the
// resulting SchedulerState (print results, collect metrics, etc.).
// ---------------------------------------------------------------------------
static void run_event_loop(SchedulerState *state, SchedulingAlgorithm algorithm) {
    Event *event_queue = initialize_events(state);

    while (event_queue != NULL) {
        Event *current = pop_event(&event_queue);
        if (current == NULL) break;

        // Advance time and record CPU progress.
        // STCF always records on arrivals (preemption check follows below).
        // All other algorithms skip recording on arrivals while the CPU is busy
        // to avoid splitting the current Gantt slice prematurely.
        if (algorithm == ALGO_STCF && current->type == EVENT_ARRIVAL) {
            record_progress(state, current->time);
            state->current_time = current->time;
        } else if (!(current->type == EVENT_ARRIVAL && state->current_process != NULL)) {
            record_progress(state, current->time);
            state->current_time = current->time;
        }

        // Discard stale completion/quantum-expire events for a process that is
        // no longer running (e.g. it was preempted after this event was queued).
        if ((current->type == EVENT_COMPLETION ||
             current->type == EVENT_QUANTUM_EXPIRE) &&
            current->process != state->current_process) {
            free(current);
            continue;
        }

        switch (current->type) {
            case EVENT_ARRIVAL:
                if (algorithm == ALGO_MLFQ) {
                    // Initialize process and place it in the highest-priority queue.
                    current->process->priority             = 0;
                    current->process->time_in_queue        = 0;
                    current->process->last_scheduled_time  = -1;
                    enqueue(&state->mlfq.queues[0], current->process);

                    // Preempt a lower-priority running process if one exists.
                    if (state->current_process != NULL &&
                        state->current_process->priority > 0) {
                        int elapsed = (state->current_process->last_scheduled_time >= 0)
                                      ? state->current_time - state->current_process->last_scheduled_time
                                      : 0;
                        state->current_process->time_in_queue += elapsed;
                        enqueue(&state->mlfq.queues[state->current_process->priority],
                                state->current_process);
                        state->current_process = NULL;
                        state->context_switches++;
                    }
                } else {
                    handle_arrival(state, current->process);
                    // STCF: preempt the running process if the newcomer is shorter.
                    if (algorithm == ALGO_STCF && state->current_process != NULL &&
                        current->process->remaining_time < state->current_process->remaining_time) {
                        handle_arrival(state, state->current_process);
                        state->current_process = NULL;
                        state->context_switches++;
                    }
                }
                break;

            case EVENT_COMPLETION:
                handle_completion(state, current->process);
                break;

            case EVENT_QUANTUM_EXPIRE:
                if (algorithm == ALGO_MLFQ)
                    handle_quantum_expire(state);
                else
                    handle_rr_quantum_expire(state);
                break;

            case EVENT_PRIORITY_BOOST:
                boost_all_priorities(state);
                break;
        }

        // Pick the next process to run if the CPU is now idle.
        if (state->current_process == NULL) {
            int status = -1;
            switch (algorithm) {
                case ALGO_FCFS: status = schedule_fcfs(state);                    break;
                case ALGO_SJF:  status = schedule_sjf(state);                     break;
                case ALGO_STCF: status = schedule_stcf(state);                    break;
                case ALGO_RR:   status = schedule_rr(state, state->quantum);      break;
                case ALGO_MLFQ: status = schedule_mlfq(state, &state->mlfq_config); break;
            }
            if (status == 0 && state->current_process != NULL) {
                if (state->current_process->start_time == -1)
                    state->current_process->start_time = state->current_time;
                state->current_process->last_scheduled_time = state->current_time;
                schedule_next_event(state, algorithm, &event_queue);
            }
        }

        if (algorithm == ALGO_MLFQ && state->mlfq.queues != NULL)
            mlfq_priority_boost(&state->mlfq, state->current_time);

        free(current);
    }
}

// ---------------------------------------------------------------------------
// Helpers for MLFQ state setup and teardown, used by both simulation paths.
// ---------------------------------------------------------------------------
static int mlfq_state_init(SchedulerState *state) {
    state->mlfq_config = createConfig();
    MLFQScheduler *sched = &state->mlfq;
    sched->num_queues   = state->mlfq_config.num_queues;
    sched->boost_period = state->mlfq_config.boost_period;
    sched->last_boost   = 0;
    sched->queues       = malloc(sizeof(MLFQQueue) * sched->num_queues);
    if (!sched->queues) return -1;
    for (int i = 0; i < sched->num_queues; i++)
        mlfq_queue_init(&sched->queues[i], i,
                        state->mlfq_config.allotments[i],
                        state->mlfq_config.time_quantums[i]);
    return 0;
}

static void mlfq_state_free(SchedulerState *state) {
    for (int i = 0; i < state->mlfq.num_queues; i++)
        free(state->mlfq.queues[i].queue);
    free(state->mlfq.queues);
    free(state->mlfq_config.time_quantums);
    free(state->mlfq_config.allotments);
}

// ---------------------------------------------------------------------------
// Public simulation entry points
// ---------------------------------------------------------------------------

// Verbose path: runs the event loop then prints the full Gantt / results.
// The caller owns the SchedulerState (including its GanttChart).
void simulate_scheduler(SchedulerState *state, SchedulingAlgorithm algorithm) {
    run_event_loop(state, algorithm);
    calculate_metrics(state->processes, state->num_processes);
    print_results(state);
}

// Silent path: builds a self-contained SchedulerState, runs the event loop,
// collects metrics, cleans up, and returns only the summary.
static SummaryMetrics simulate_scheduler_silent(Process *process_list, int count,
                                                SchedulingAlgorithm algorithm,
                                                int quantum) {
    SchedulerState state;
    memset(&state, 0, sizeof(SchedulerState));

    GanttChart gantt;
    gantt_init(&gantt);
    state.gantt         = &gantt;
    state.processes     = process_list;
    state.num_processes = count;
    state.quantum       = quantum;

    if (algorithm == ALGO_MLFQ) {
        if (mlfq_state_init(&state) != 0) {
            fprintf(stderr, "Error: out of memory in simulate_scheduler_silent\n");
            exit(1);
        }
    }

    run_event_loop(&state, algorithm);
    calculate_metrics(state.processes, state.num_processes);

    SummaryMetrics m;
    m.avg_turnaround   = avg_turnaround_time(state.processes, state.num_processes);
    m.avg_waiting      = avg_waiting_time(state.processes, state.num_processes);
    m.avg_response     = avg_response_time(state.processes, state.num_processes);
    m.context_switches = state.context_switches;

    // Cleanup
    gantt_free(&gantt);
    if (state.ready_queue) free(state.ready_queue);
    if (algorithm == ALGO_MLFQ) mlfq_state_free(&state);

    return m;
}

// ---------------------------------------------------------------------------
// Comparison mode: runs all five algorithms and prints a summary table.
// ---------------------------------------------------------------------------
static void run_comparison(const char *input_file, char *process_str, int quantum) {
    const char *source = input_file ? input_file : "(process string)";
    printf("=== Algorithm Comparison for %s ===\n\n", source);
    printf("%-12s | %8s | %8s | %8s | %s\n",
           "Algorithm", "Avg TT", "Avg WT", "Avg RT", "Context Switches");
    printf("%-12s-+-%8s-+-%8s-+-%8s-+-----------------\n",
           "------------", "--------", "--------", "--------");

    static const struct { const char *label; SchedulingAlgorithm algo; int uses_quantum; } algos[] = {
        { "FCFS",  ALGO_FCFS,  0 },
        { "SJF",   ALGO_SJF,   0 },
        { "STCF",  ALGO_STCF,  0 },
        { "RR",    ALGO_RR,    1 },
        { "MLFQ",  ALGO_MLFQ,  0 },
    };
    int n_algos = (int)(sizeof(algos) / sizeof(algos[0]));

    for (int i = 0; i < n_algos; i++) {
        int count = 0;
        Process *processes = input_file ? loadProcesses(input_file, &count)
                                        : parseProcessString(process_str, &count);
        if (!processes || count == 0) {
            fprintf(stderr, "Error: failed to load processes for %s\n", algos[i].label);
            continue;
        }

        SummaryMetrics m = simulate_scheduler_silent(processes, count,
                                                     algos[i].algo, quantum);
        free(processes);

        char label[24];
        if (algos[i].uses_quantum)
            snprintf(label, sizeof(label), "%s (q=%d)", algos[i].label, quantum);
        else
            snprintf(label, sizeof(label), "%s", algos[i].label);

        printf("%-12s | %8.1f | %8.1f | %8.1f | %d\n",
               label,
               m.avg_turnaround, m.avg_waiting, m.avg_response, m.context_switches);
    }
    printf("\n");
}

// ---------------------------------------------------------------------------
// main
// ---------------------------------------------------------------------------
int main(int argc, char *argv[]) {
    char *input_file  = NULL;
    char *process_str = NULL;
    char *algo_name   = NULL;
    int   quantum     = 30;
    int   compare     = 0;

    for (int i = 1; i < argc; i++) {
        if      (strcmp(argv[i], "--compare")         == 0) compare     = 1;
        else if (strncmp(argv[i], "--input=",     8)  == 0) input_file  = argv[i] + 8;
        else if (strncmp(argv[i], "--processes=", 12) == 0) process_str = argv[i] + 12;
        else if (strncmp(argv[i], "--algorithm=", 12) == 0) algo_name   = argv[i] + 12;
        else if (strncmp(argv[i], "--quantum=",   10) == 0) quantum     = atoi(argv[i] + 10);
    }

    if (compare) {
        if (!input_file && !process_str) {
            fprintf(stderr, "Usage: %s --compare (--input=<file> | --processes=\"...\") [--quantum=<n>]\n", argv[0]);
            return 1;
        }
        run_comparison(input_file, process_str, quantum);
        return 0;
    }

    if (!algo_name || (!input_file && !process_str)) {
        fprintf(stderr, "Usage: %s --algorithm=<ALGO> (--input=<file> | --processes=\"PID:Arr:Burst,...\") [--quantum=<n>]\n", argv[0]);
        fprintf(stderr, "       %s --compare          (--input=<file> | --processes=\"PID:Arr:Burst,...\") [--quantum=<n>]\n", argv[0]);
        return 1;
    }

    int count = 0;
    Process *process_list = input_file ? loadProcesses(input_file, &count)
                                       : parseProcessString(process_str, &count);
    if (!process_list || count == 0) {
        fprintf(stderr, "Error: No processes loaded.\n");
        return 1;
    }

    SchedulerState state;
    memset(&state, 0, sizeof(SchedulerState));

    state.gantt = malloc(sizeof(GanttChart));
    if (!state.gantt) {
        fprintf(stderr, "Error: out of memory allocating GanttChart\n");
        free(process_list);
        return 1;
    }
    gantt_init(state.gantt);

    state.processes     = process_list;
    state.num_processes = count;
    state.quantum       = quantum;

    SchedulingAlgorithm selected = get_algorithm_type(algo_name);
    if ((int)selected == -1) { free(process_list); free(state.gantt); return 1; }

    if (selected == ALGO_MLFQ) {
        if (mlfq_state_init(&state) != 0) {
            fprintf(stderr, "Error: out of memory allocating MLFQ queues\n");
            free(process_list);
            free(state.gantt);
            return 1;
        }
    }

    printf("Running %s Scheduler...\n\n", algo_name);
    simulate_scheduler(&state, selected);

    gantt_free(state.gantt);
    free(state.gantt);
    free(process_list);
    if (state.ready_queue) free(state.ready_queue);
    if (selected == ALGO_MLFQ) mlfq_state_free(&state);

    return 0;
}