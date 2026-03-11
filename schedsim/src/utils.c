#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../include/process.h"
#include "../include/scheduler.h"

Process *parse_processes_from_string(const char *s, int *out_count) {
    int capacity = 8, count = 0;
    Process *procs = malloc(capacity * sizeof(Process));

    // manual copy instead of strdup (not in C99)
    char *buf = malloc(strlen(s) + 1);
    strcpy(buf, s);

    char *tok = strtok(buf, ",");
    while (tok) {
        if (count >= capacity) {
            capacity *= 2;
            procs = realloc(procs, capacity * sizeof(Process));
        }
        char pid[16]; int at, bt;
        if (sscanf(tok, "%15[^:]:%d:%d", pid, &at, &bt) == 3) {
            memset(&procs[count], 0, sizeof(Process));
            strncpy(procs[count].pid, pid, 15);
            procs[count].arrival_time   = at;
            procs[count].burst_time     = bt;
            procs[count].remaining_time = bt;
            procs[count].start_time     = -1;
            procs[count].finish_time    = -1;
            count++;
        }
        tok = strtok(NULL, ",");
    }
    free(buf);
    *out_count = count;
    return procs;
}