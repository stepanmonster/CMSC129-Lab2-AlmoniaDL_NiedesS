# CMSC129 Lab 2 — CPU Scheduling Simulator

## Group Members
- **Almonia, D. L.**
- **Niedes, S.**

---

## Overview

A command-line CPU scheduling simulator implementing FCFS, SJF, STCF, Round Robin, and MLFQ. Outputs per-process metrics and ASCII Gantt charts.

---

## Implemented Algorithms

| Algorithm | Flag | Notes |
|---|---|---|
| First-Come, First-Served | `FCFS` | Non-preemptive |
| Shortest Job First | `SJF` | Non-preemptive |
| Shortest Time-to-Completion First | `STCF` | Preemptive SJF |
| Round Robin | `RR` | Configurable time quantum via `--quantum` |
| Multi-Level Feedback Queue | `MLFQ` | Allotment-based demotion, priority boost |

---

## Prerequisites

- **GCC** 9 or later (C11 support required)
- **Make** 4.0 or later
- **Bash** (for running the test suite on Linux/macOS)
- **Valgrind** *(optional, for memory leak checking)*

---

## Compilation

```bash
make clean      # Remove previous build artifacts
make all        # Build the main executable and test suite
```

The compiled binary will be placed at `./bin/scheduler`.

---

## Usage

```bash
# Run with an input file
./bin/scheduler --algorithm <ALGO> --input <FILE>

# Run with an inline process list
./bin/scheduler --algorithm <ALGO> --processes "<PROCESS_LIST>" [OPTIONS]

# Compare all algorithms on the same workload
./bin/scheduler --compare --input <FILE>
```

### Options

| Flag | Description |
|---|---|
| `--algorithm <ALGO>` | Scheduling algorithm: `FCFS`, `SJF`, `STCF`, `RR`, `MLFQ` |
| `--input <FILE>` | Path to workload file |
| `--processes "<LIST>"` | Inline process list (e.g., `"P1,0,5 P2,2,3"`) |
| `--quantum <N>` | Time quantum for Round Robin (default: 1) |
| `--compare` | Run all four classic algorithms and display side-by-side results |

### Process List Format

Each process entry follows `<Name>,<ArrivalTime>,<BurstTime>`, separated by spaces:

```
"P1,0,5 P2,2,3 P3,4,1"
```

### Workload File Format

One process per line:

```
P1 0 5
P2 2 3
P3 4 1
```

---

## Example Usage

```bash
# FCFS from file
./bin/scheduler --algorithm FCFS --input tests/workload1.txt

# Round Robin with quantum=2
./bin/scheduler --algorithm RR --processes "P1,0,6 P2,1,4 P3,3,2" --quantum 2

# MLFQ from file
./bin/scheduler --algorithm MLFQ --input tests/workload1.txt

# Compare all algorithms
./bin/scheduler --compare --input tests/workload1.txt
```

### Sample Output

```
Algorithm: FCFS

Process  AT   BT   FT   TT   WT
P1        0    5    5    5    0
P2        2    3    8    6    3
P3        4    1    9    5    4

Average Turnaround Time : 5.33
Average Waiting Time    : 2.33

Gantt Chart:
| P1 | P1 | P1 | P1 | P1 | P2 | P2 | P2 | P3 |
0    1    2    3    4    5    6    7    8    9
```

---

## Metrics

| Metric | Formula |
|---|---|
| Turnaround Time (TT) | `FT − AT` |
| Waiting Time (WT) | `TT − BT` |
| Finish Time (FT) | Time the process completes execution |

---

## Running Tests

```bash
bash tests/test_suite.sh
```

The test suite verifies all algorithms against lecture-expected values and checks edge cases (single process, simultaneous arrivals, identical burst times).

To check for memory leaks:

```bash
valgrind --leak-check=full ./bin/scheduler --algorithm FCFS --input tests/workload1.txt
```

---

## Project Structure

```
.
├── bin/                  # Compiled binaries
├── src/                  # Source files
│   ├── main.c
│   ├── fcfs.c
│   ├── sjf.c
│   ├── stcf.c
│   ├── rr.c
│   ├── mlfq.c
│   ├── gantt.c
│   └── metrics.c
├── include/              # Header files
├── tests/
│   ├── workload1.txt
│   └── test_suite.sh
├── docs/
│   └── mlfq_design.md
├── Makefile
└── README.md
```

---

## Known Limitations & Assumptions

- MLFQ scheduling decisions are based entirely on observed runtime behavior; `BurstTime` is never read by the MLFQ scheduler.
- The Gantt chart uses ASCII rendering with automatic scaling for long simulations.
- Context switch overhead is counted but not added to simulation time.
- All burst times and arrival times must be non-negative integers.
- `--compare` mode runs FCFS, SJF, STCF, and RR only (not MLFQ).