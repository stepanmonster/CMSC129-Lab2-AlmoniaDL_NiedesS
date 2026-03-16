#ifndef METRICS_H
#define METRICS_H

#include "process.h"

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