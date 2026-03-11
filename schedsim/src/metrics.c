#include <stdio.h>
#include "../include/metrics.h"
#include "../include/process.h"

// turnaround_time is not stored in Process; compute on the fly
static int turnaround(Process *p) { return p->finish_time - p->arrival_time; }
static int response(Process *p)   { return p->start_time  - p->arrival_time; }

void calculate_metrics(Process *processes, int n) {
    for (int i = 0; i < n; i++) {
        Process *p = &processes[i];
        // waiting_time = TT - BT
        p->waiting_time = turnaround(p) - p->burst_time;
    }
}

void print_metrics_table(Process *processes, int n) {
    printf("Process | AT  | BT  | FT  | TT  | WT  | RT\n");
    printf("--------|-----|-----|-----|-----|-----|-----\n");

    double sum_tt = 0, sum_wt = 0, sum_rt = 0;
    for (int i = 0; i < n; i++) {
        Process *p = &processes[i];
        int tt = turnaround(p);
        int rt = response(p);
        printf("%-7s | %3d | %3d | %3d | %3d | %3d | %3d\n",
               p->pid, p->arrival_time, p->burst_time,
               p->finish_time, tt, p->waiting_time, rt);
        sum_tt += tt;
        sum_wt += p->waiting_time;
        sum_rt += rt;
    }
    printf("--------|-----|-----|-----|-----|-----|-----\n");
    printf("Average |     |     |     | %5.1f | %5.1f | %5.1f\n",
           sum_tt / n, sum_wt / n, sum_rt / n);
}

void print_summary(SummaryMetrics *summary) {
    printf("Avg Turnaround:   %.2f\n", summary->avg_turnaround);
    printf("Avg Waiting:      %.2f\n", summary->avg_waiting);
    printf("Avg Response:     %.2f\n", summary->avg_response);
    printf("Context Switches: %d\n",  summary->context_switches);
}

double avg_turnaround_time(Process *processes, int n) {
    double sum = 0;
    for (int i = 0; i < n; i++) sum += turnaround(&processes[i]);
    return sum / n;
}

double avg_waiting_time(Process *processes, int n) {
    double sum = 0;
    for (int i = 0; i < n; i++) sum += processes[i].waiting_time;
    return sum / n;
}

double avg_response_time(Process *processes, int n) {
    double sum = 0;
    for (int i = 0; i < n; i++) sum += response(&processes[i]);
    return sum / n;
}