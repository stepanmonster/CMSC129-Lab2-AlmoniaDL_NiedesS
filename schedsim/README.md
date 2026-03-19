# CMSC129-Lab2-AlmoniaDL_NiedesS

## Project Timeline

### Week 1 — Foundation & Core Algorithms
**Goal:** Get the project compiling and implement the simpler scheduling algorithms.

- Set up project structure (`src/`, `include/`, `tests/`, `docs/`), Makefile, and README skeleton
- Implement core data structures (`Process`, `SchedulerState`, `MLFQQueue`, event queue)
- Build CLI argument parser (`--algorithm`, `--input`, `--processes`, `--quantum`)
- Implement workload file parser and `--processes` string parser
- Implement and test **FCFS** and **SJF**
- Implement metrics calculation (`calculate_metrics()`) and basic table output
- Write initial test workloads (`tests/workload1.txt`)

---

### Week 2 — Preemptive Algorithms & Gantt Charts
**Goal:** Complete all four classic algorithms and add visual output.

- Implement **STCF** with preemption logic and remaining-time tracking
- Implement **Round Robin** with configurable time quantum and context switch counting
- Build ASCII Gantt chart renderer (`gantt.c`) with scaling for long simulations
- Add per-process metric breakdown output (showing formulas: `TT = FT - AT`, etc.)
- Implement `--compare` mode to run all four algorithms on the same workload
- Write and run edge case tests (single process, simultaneous arrivals, identical burst times)

---

### Week 3 — MLFQ, Polish & Documentation
**Goal:** Complete MLFQ, finalize testing, and write all required documentation.

- Design and implement **MLFQ** (`mlfq.c`) — queues, allotment tracking, demotion logic
- Implement priority boost mechanism (period S, anti-gaming via allotment reset)
- Verify MLFQ never reads `BurstTime` — scheduling decisions based on observed behavior only
- Write `docs/mlfq_design.md` with parameter justification and empirical test results
- Write `tests/test_suite.sh` automating verification against lecture-expected values
- Final integration testing: all algorithms, `--compare`, edge cases, memory checks
- Complete `README.md` with usage examples, known limitations, and sample output