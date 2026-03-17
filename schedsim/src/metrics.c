#include <stdio.h>
#include "../include/process.h"
#include "../include/metrics.h"

void calculate_metrics(Process *processes, int n) {
    for (int i = 0; i < n; i++) {
        Process *p = &processes[i];
        p->turnaround_time = p->finish_time - p->arrival_time;
        p->waiting_time    = p->turnaround_time - p->burst_time;
        p->response_time   = p->start_time - p->arrival_time;
    }
}

double avg_turnaround_time(Process *processes, int n) {
    double sum = 0.0;
    for (int i = 0; i < n; i++) sum += processes[i].turnaround_time;
    return sum / n;
}

double avg_waiting_time(Process *processes, int n) {
    double sum = 0.0;
    for (int i = 0; i < n; i++) sum += processes[i].waiting_time;
    return sum / n;
}

double avg_response_time(Process *processes, int n) {
    double sum = 0.0;
    for (int i = 0; i < n; i++) sum += processes[i].response_time;
    return sum / n;
}

void print_metrics_table(Process *processes, int n) {
    printf("%-10s %10s %10s %10s %10s %10s\n",
           "PID", "Arrival", "Burst", "Finish", "Turnaround", "Waiting");
    printf("%-10s %10s %10s %10s %10s %10s\n",
           "---", "-------", "-----", "------", "----------", "-------");
    for (int i = 0; i < n; i++) {
        Process *p = &processes[i];
        printf("%-10s %10d %10d %10d %10d %10d\n",
               p->pid, p->arrival_time, p->burst_time,
               p->finish_time, p->turnaround_time, p->waiting_time);
    }
    printf("\nAverage Turnaround : %.2f\n", avg_turnaround_time(processes, n));
    printf("Average Waiting    : %.2f\n",   avg_waiting_time(processes, n));
    printf("Average Response   : %.2f\n",   avg_response_time(processes, n));
}

void print_summary(SummaryMetrics *s) {
    printf("\n=== Summary ===\n");
    printf("Avg Turnaround : %.2f\n", s->avg_turnaround);
    printf("Avg Waiting    : %.2f\n", s->avg_waiting);
    printf("Avg Response   : %.2f\n", s->avg_response);
    printf("Context Switches: %d\n",  s->context_switches);
}