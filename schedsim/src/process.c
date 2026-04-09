#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "../include/process.h"
#include "../include/scheduler.h"

#define ARGCOUNT 3

Process parseProcess(char *input)
{
    Process p;
    memset(&p, 0, sizeof(Process));
    char *args[ARGCOUNT];
    int i = 0;

    char *token = strtok(input, " \n");
    while (token != NULL && i < ARGCOUNT)
    {
        args[i++] = token;
        token = strtok(NULL, " \n");
    }

    if (i == ARGCOUNT)
    {
        strncpy(p.pid, args[0], sizeof(p.pid) - 1);
        p.pid[sizeof(p.pid) - 1] = '\0';
        p.arrival_time = atoi(args[1]);
        p.burst_time = atoi(args[2]);
        p.remaining_time = p.burst_time;
        p.start_time = -1;           // Not started yet
        p.finish_time = -1;          // Not finished yet
        p.priority = 0;              // Highest priority initially
        p.time_in_queue = 0;         // Time spent in current queue
        p.last_scheduled_time = -1;  // Never scheduled
        p.waiting_time = 0;
        p.turnaround_time = 0;
        p.response_time = 0;
    }
    return p;
}

Process* loadProcesses(const char *filename, int *out_count)
{
    int capacity = 10;
    int count = 0;
    Process *processes = malloc(capacity * sizeof(Process));
    if (!processes) {
        fprintf(stderr, "Error: out of memory in loadProcesses\n");
        return NULL;
    }

    FILE *file = fopen(filename, "r");
    if (!file) {
        perror(filename);
        free(processes);
        return NULL;
    }

    char line[100];
    while (fgets(line, sizeof(line), file)) {
        if (line[0] == '\n' || line[0] == '#') continue;

        if (count >= capacity) {
            capacity *= 2;
            Process *tmp = realloc(processes, capacity * sizeof(Process));
            if (!tmp) {
                fprintf(stderr, "Error: out of memory in loadProcesses\n");
                fclose(file);
                free(processes);
                return NULL;
            }
            processes = tmp;
        }

        processes[count++] = parseProcess(line);
    }

    fclose(file);
    *out_count = count;
    return processes;
}

Process* parseProcessString(char *input, int *out_count) {
    int capacity = 10;
    int count = 0;
    Process *processes = malloc(capacity * sizeof(Process));
    if (!processes) {
        fprintf(stderr, "Error: out of memory in parseProcessString\n");
        return NULL;
    }

    // Use malloc + strcpy since we aren't using strdup
    char *input_copy = malloc(strlen(input) + 1);
    if (!input_copy) {
        free(processes);
        return NULL;
    }
    strcpy(input_copy, input);

    // Outer loop: Split by commas
    char *process_token = strtok(input_copy, ",");
    while (process_token != NULL) {
        if (count >= capacity) {
            capacity *= 2;
            Process *tmp = realloc(processes, capacity * sizeof(Process));
            if (!tmp) {
                fprintf(stderr, "Error: out of memory in parseProcessString\n");
                free(input_copy);
                free(processes);
                return NULL;
            }
            processes = tmp;
        }

        Process p;
        memset(&p, 0, sizeof(Process));

        // Use sscanf to parse "PID:Arrival:Burst" directly from the token
        // %[^:] means "read everything that isn't a colon"
        if (sscanf(process_token, "%[^:]:%d:%d", p.pid, &p.arrival_time, &p.burst_time) == 3) {
            p.remaining_time = p.burst_time;
            p.start_time = -1;
            p.finish_time = -1;
            p.priority = 0;
            p.last_scheduled_time = -1;

            processes[count++] = p;
        }

        // Now strtok correctly finds the NEXT comma in input_copy
        process_token = strtok(NULL, ",");
    }

    free(input_copy);
    *out_count = count;
    return processes;
}