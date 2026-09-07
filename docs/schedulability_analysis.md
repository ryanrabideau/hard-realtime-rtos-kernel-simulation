# Schedulability Analysis

This document analyzes the three-task periodic workload used by the RTOS simulation.

The task set is:

- Task 1: priority 3, execution time C = 30 ticks, period T = 50 ticks
- Task 2: priority 8, execution time C = 8 ticks, period T = 100 ticks
- Task 3: priority 15, execution time C = 6 ticks, period T = 150 ticks

Lower numeric values represent higher scheduling priorities. Since the tasks with shorter periods are assigned higher priorities, the ordering follows Rate Monotonic scheduling.

For this analysis, each task's relative deadline is equal to its period.

## Processor Utilization

Total processor utilization is calculated as:

```text
U = C1/T1 + C2/T2 + C3/T3
```

Substituting the task parameters:

```text
U = 30/50 + 8/100 + 6/150
U = 0.60 + 0.08 + 0.04
U = 0.72
```

The total utilization of the task set is therefore 72%.

## Liu & Layland Bound

For n periodic tasks scheduled using Rate Monotonic priorities, the sufficient utilization bound is:

```text
U_bound = n(2^(1/n) - 1)
```

For three tasks:

```text
U_bound = 3(2^(1/3) - 1)
U_bound ≈ 0.7798
```

The task set has:

```text
U = 0.72
```

Since:

```text
0.72 < 0.7798
```

the task set satisfies the Liu & Layland sufficient schedulability test.

## Response-Time Analysis

Response-time analysis provides another way to check whether each task can complete before its deadline.

For a task i, the response time is found iteratively using:

```text
Ri = Ci + sum(ceil(Ri / Tj) * Cj)
```

where the sum includes all tasks with higher priority than task i.

### Task 1

Task 1 has the highest priority, so it experiences no interference from higher-priority tasks.

```text
R1 = C1
R1 = 30 ticks
```

Its deadline is 50 ticks:

```text
30 <= 50
```

Task 1 meets its deadline.

### Task 2

Task 2 can be interrupted by Task 1.

Start with:

```text
R2(0) = C2 = 8
```

First iteration:

```text
R2(1) = 8 + ceil(8/50) * 30
R2(1) = 8 + 30
R2(1) = 38
```

Second iteration:

```text
R2(2) = 8 + ceil(38/50) * 30
R2(2) = 38
```

The result has converged:

```text
R2 = 38 ticks
```

Its deadline is 100 ticks:

```text
38 <= 100
```

Task 2 meets its deadline.

### Task 3

Task 3 can be interrupted by both Task 1 and Task 2.

Start with:

```text
R3(0) = C3 = 6
```

First iteration:

```text
R3(1) = 6
        + ceil(6/50) * 30
        + ceil(6/100) * 8

R3(1) = 6 + 30 + 8
R3(1) = 44
```

Second iteration:

```text
R3(2) = 6
        + ceil(44/50) * 30
        + ceil(44/100) * 8

R3(2) = 44
```

The result has converged:

```text
R3 = 44 ticks
```

Its deadline is 150 ticks:

```text
44 <= 150
```

Task 3 meets its deadline.

## Results

The calculated worst-case response times are:

- Task 1: 30 ticks with a 50-tick deadline
- Task 2: 38 ticks with a 100-tick deadline
- Task 3: 44 ticks with a 150-tick deadline

All three calculated response times are below their respective deadlines.

The task set therefore passes both the Liu & Layland utilization test and the response-time analysis for the timing parameters used by the simulation.

## Notes

The execution times used here are simulation parameters rather than measurements of CPU execution time on physical hardware.

The program runs as a deterministic C11 userspace simulation, so terminal output and host operating-system execution time are not used as real-time timing measurements. The tick values instead represent the timing model being analyzed by the kernel simulation.
