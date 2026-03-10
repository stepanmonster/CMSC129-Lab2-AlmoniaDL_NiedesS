#ifndef GANTT_H
#define GANTT_H

#include "process.h"

typedef struct {
    char pid[16];   // which process ran
    int start;      // when it started this slice
    int end;        // when this slice ended
} GanttEntry;

typedef struct {
    GanttEntry *entries;
    int count;
    int capacity;
} GanttChart;

void gantt_init(GanttChart *g);
void gantt_add(GanttChart *g, const char *pid, int start, int end);
void gantt_print(GanttChart *g);
void gantt_free(GanttChart *g);

#endif