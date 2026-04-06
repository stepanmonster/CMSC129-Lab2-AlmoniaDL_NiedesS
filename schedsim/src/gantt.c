#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../include/gantt.h"

// ─── Initialize empty gantt chart ────────────────────────────────────
void gantt_init(GanttChart *g) {
    g->capacity = 64;
    g->count    = 0;
    g->entries  = malloc(sizeof(GanttEntry) * g->capacity);
    if (!g->entries) {
        fprintf(stderr, "Error: out of memory in gantt_init\n");
        exit(1);
    }
}

// ─── Add a new entry ─────────────────────────────────────────────────
void gantt_add(GanttChart *g, const char *pid, int start, int end) {
    // grow if needed
    if (g->count >= g->capacity) {
        g->capacity *= 2;
        GanttEntry *tmp = realloc(g->entries, sizeof(GanttEntry) * g->capacity);
        if (!tmp) {
            fprintf(stderr, "Error: out of memory in gantt_add\n");
            exit(1);
        }
        g->entries = tmp;
    }

    strncpy(g->entries[g->count].pid, pid, 15);
    g->entries[g->count].start = start;
    g->entries[g->count].end   = end;
    g->count++;
}

// ─── Print the gantt chart ───────────────────────────────────────────
void gantt_print(GanttChart *g) {
    if (g->count == 0) {
        printf("(empty)\n");
        return;
    }

    int total_time = g->entries[g->count - 1].end;

    // ── scale if too long ──
    // each char = 1 unit if total <= 80, else scale up
    int scale = 1;
    if (total_time > 80) {
        scale = (total_time / 80) + 1;
        printf("(scaled: each char = %d time units)\n", scale);
    }

    // ── top bar: [A----][B---][C--] ──
    for (int i = 0; i < g->count; i++) {
        int width = (g->entries[i].end - g->entries[i].start) / scale;
        if (width < 1) width = 1;   // minimum width of 1

        printf("[%s", g->entries[i].pid);
        for (int j = 0; j < width - 1; j++) printf("-");
        printf("]");
    }
    printf("\n");

    // ── time markers below ──
    // print start time of first entry
    printf("%-4d", g->entries[0].start);

    for (int i = 0; i < g->count; i++) {
        int width = (g->entries[i].end - g->entries[i].start) / scale;
        if (width < 1) width = 1;

        // [ + pid + dashes(width-1) + ] = 1 + strlen(pid) + (width-1) + 1 = width + strlen(pid) + 1
        int pad = width + (int)strlen(g->entries[i].pid) + 1;
        printf("%*d", pad, g->entries[i].end);
    }
    printf("\n");
}

// ─── Free memory ─────────────────────────────────────────────────────
void gantt_free(GanttChart *g) {
    free(g->entries);
    g->entries  = NULL;
    g->count    = 0;
    g->capacity = 0;
}