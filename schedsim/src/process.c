#define _POSIX_C_SOURCE 200809L

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <limits.h>

#include "../include/process.h"
#include "../include/scheduler.h"

/* -------------------------------------------------------------------------
 * parse_int: validated integer parse using strtol.
 * Returns 0 on success (value written to *out).
 * Returns -1 and prints to stderr on overflow, underflow, or non-numeric.
 * min_val lets callers reject negative numbers.
 * ------------------------------------------------------------------------- */
static int parse_int(const char *str, int min_val, const char *field_name, int *out)
{
    if (!str || str[0] == '\0') {
        fprintf(stderr, "Error: %s is missing or empty\n", field_name);
        return -1;
    }

    char *endptr;
    errno = 0;
    long val = strtol(str, &endptr, 10);

    if (errno == ERANGE || val > INT_MAX || val < INT_MIN) {
        fprintf(stderr, "Error: %s value '%s' is out of range\n", field_name, str);
        return -1;
    }
    if (endptr == str || (*endptr != '\0' && *endptr != '\n' && *endptr != '\r')) {
        fprintf(stderr, "Error: %s value '%s' is not a valid integer\n", field_name, str);
        return -1;
    }
    if ((int)val < min_val) {
        fprintf(stderr, "Error: %s value %ld is negative (minimum is %d)\n",
                field_name, val, min_val);
        return -1;
    }

    *out = (int)val;
    return 0;
}

/* -------------------------------------------------------------------------
 * init_process_fields: set all runtime fields to clean initial values.
 * ------------------------------------------------------------------------- */
static void init_process_fields(Process *p)
{
    p->remaining_time     = p->burst_time;
    p->start_time         = -1;
    p->finish_time        = -1;
    p->priority           = 0;
    p->time_in_queue      = 0;
    p->last_scheduled_time = -1;
    p->waiting_time       = 0;
    p->turnaround_time    = 0;
    p->response_time      = 0;
}

/* -------------------------------------------------------------------------
 * parseProcess (formerly parseInput):
 *   Parses one line of the form "PID ArrivalTime BurstTime".
 *   Returns a zeroed Process{} on parse failure so the caller can detect
 *   the error by checking pid[0] == '\0'.
 * ------------------------------------------------------------------------- */
Process parseProcess(char *input)
{
    Process p;
    memset(&p, 0, sizeof(Process));

    char *saveptr;
    char *pid_tok = strtok_r(input, " \t\n\r", &saveptr);
    char *at_tok  = strtok_r(NULL,  " \t\n\r", &saveptr);
    char *bt_tok  = strtok_r(NULL,  " \t\n\r", &saveptr);

    if (!pid_tok || !at_tok || !bt_tok) {
        fprintf(stderr, "Error: process line must have three fields: PID ArrivalTime BurstTime\n");
        return p;   /* zeroed — pid[0] == '\0' signals failure */
    }

    strncpy(p.pid, pid_tok, sizeof(p.pid) - 1);
    p.pid[sizeof(p.pid) - 1] = '\0';

    if (parse_int(at_tok, 0, "ArrivalTime", &p.arrival_time) != 0) { p.pid[0] = '\0'; return p; }
    if (parse_int(bt_tok, 1, "BurstTime",   &p.burst_time)   != 0) { p.pid[0] = '\0'; return p; }

    init_process_fields(&p);
    return p;
}

/* -------------------------------------------------------------------------
 * loadProcesses: read processes from a file (one per line).
 * ------------------------------------------------------------------------- */
Process *loadProcesses(const char *filename, int *out_count)
{
    int capacity = 10;
    int count    = 0;
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

    char line[128];
    while (fgets(line, sizeof(line), file)) {
        /* Skip blank lines and comments */
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

        Process p = parseProcess(line);
        if (p.pid[0] == '\0') {
            /* parseProcess already printed an error */
            fclose(file);
            free(processes);
            return NULL;
        }
        processes[count++] = p;
    }

    fclose(file);
    *out_count = count;
    return processes;
}

/* -------------------------------------------------------------------------
 * parseProcessString:
 *   Parses the README-documented inline format:
 *     "P1,0,5 P2,2,3 P3,4,1"
 *   Each space-separated token is <PID>,<ArrivalTime>,<BurstTime>.
 *
 *   (The old implementation used colon delimiters; this matches the docs.)
 * ------------------------------------------------------------------------- */
Process *parseProcessString(char *input, int *out_count)
{
    int capacity = 10;
    int count    = 0;
    Process *processes = malloc(capacity * sizeof(Process));
    if (!processes) {
        fprintf(stderr, "Error: out of memory in parseProcessString\n");
        return NULL;
    }

    char *input_copy = malloc(strlen(input) + 1);
    if (!input_copy) {
        free(processes);
        return NULL;
    }
    strcpy(input_copy, input);

    char *saveptr_outer;
    /* Split by spaces to get individual "PID,AT,BT" tokens */
    char *token = strtok_r(input_copy, " \t", &saveptr_outer);
    while (token != NULL) {
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

        /* Each token has the form PID,ArrivalTime,BurstTime */
        char *saveptr_inner;
        char *pid_tok = strtok_r(token, ",", &saveptr_inner);
        char *at_tok  = strtok_r(NULL,  ",", &saveptr_inner);
        char *bt_tok  = strtok_r(NULL,  ",", &saveptr_inner);

        if (!pid_tok || !at_tok || !bt_tok) {
            fprintf(stderr,
                    "Error: malformed process token '%s' — expected PID,ArrivalTime,BurstTime\n",
                    token);
            free(input_copy);
            free(processes);
            return NULL;
        }

        strncpy(p.pid, pid_tok, sizeof(p.pid) - 1);
        p.pid[sizeof(p.pid) - 1] = '\0';

        if (parse_int(at_tok, 0, "ArrivalTime", &p.arrival_time) != 0 ||
            parse_int(bt_tok, 1, "BurstTime",   &p.burst_time)   != 0) {
            free(input_copy);
            free(processes);
            return NULL;
        }

        init_process_fields(&p);
        processes[count++] = p;

        token = strtok_r(NULL, " \t", &saveptr_outer);
    }

    free(input_copy);
    *out_count = count;
    return processes;
}