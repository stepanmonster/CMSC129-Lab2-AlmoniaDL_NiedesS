#ifndef METRICS_H
#define METRICS_H

#include "process.h"

// Per-process metrics
typedef struct {
    char pid[16];
    int arrival_time;   // AT: when process arrived
    int burst_time;     // BT: total CPU time needed
    int finish_time;    // FT: when process completed
    int turnaround_time; // TT = FT - AT
    int waiting_time;   // WT = TT - BT
    int response_time;  // RT = start_time - AT
} ProcessMetrics;

// Summary metrics across all processes
typedef struct {
    double avg_turnaround;  // average TT
    double avg_waiting;     // average WT
    double avg_response;    // average RT
    int context_switches;
} SummaryMetrics;

// Function declarations
void calculate_metrics(Process *processes, int n);
void print_metrics_table(Process *processes, int n);
void print_summary(SummaryMetrics *summary);

double avg_turnaround_time(Process *processes, int n);
double avg_waiting_time(Process *processes, int n);
double avg_response_time(Process *processes, int n);

#endif