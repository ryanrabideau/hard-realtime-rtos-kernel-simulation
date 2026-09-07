# Hard RTOS Kernel Simulation in C

A deterministic hard real-time RTOS kernel simulation written in pure C11 and executed in userspace on Linux. Built to explore core real-time systems concepts including fixed-priority scheduling, priority inheritance, static memory management, deadline monitoring, watchdog fault detection, and schedulability analysis.

## Project Overview

This project implements a small RTOS-style kernel that runs deterministically in a single-threaded simulation loop. The goal is to build and test the scheduling and timing concepts behind a hard real-time system without relying on an existing RTOS, operating system threads, or dynamic memory allocation.

The kernel includes:
- Fixed-priority Rate Monotonic scheduling
- Priority-inheritance mutexes
- Static task and stack allocation
- Deadline monitoring
- Watchdog stall detection
- Stack overflow detection
- Deterministic task state machines
- Liu & Layland utilization analysis
- Worst-case response-time analysis

This is a userspace simulation rather than a hardware RTOS. It does not perform real CPU context switching or interrupt-driven scheduling. Instead, task execution and preemption are modeled deterministically so the scheduling behavior can be tested and inspected directly.

## Task Set

The main simulation uses three periodic tasks:

- Task 1: priority 3, period 50 ticks, WCET 30 ticks
- Task 2: priority 8, period 100 ticks, WCET 8 ticks
- Task 3: priority 15, period 150 ticks, WCET 6 ticks

Lower numbers represent higher priorities. Since the shorter-period tasks are assigned higher priorities, the task set follows Rate Monotonic scheduling.

## Scheduling

The scheduler selects the highest-priority task that is currently in the READY state. Tasks are implemented as state machines, so each call performs a bounded amount of simulated work before returning control to the kernel.

This keeps execution deterministic and makes scheduling behavior easy to reproduce without depending on Linux thread scheduling.

## Priority Inheritance

The kernel includes a mutex with priority inheritance to demonstrate how priority inversion can be handled.

A dedicated test scenario creates the following sequence:

1. The low-priority task acquires the shared mutex.
2. The high-priority task attempts to acquire it and becomes blocked.
3. The low-priority owner temporarily inherits the high-priority task's priority.
4. The medium-priority task becomes ready but cannot preempt the boosted mutex owner.
5. The low-priority task finishes its critical section and releases the mutex.
6. Its original priority is restored.
7. The high-priority task is awakened and acquires the mutex.

The current mutex implementation intentionally models one blocked waiter rather than a full production RTOS wait queue.

A successful test ends with:

```text
PASS: priority inheritance resolved inversion correctly
```

## Static Memory

The kernel does not use `malloc()` or `free()`.

Memory is allocated statically using:
- A fixed array supporting up to 10 task control blocks
- A 512-byte simulated stack for each task
- An `0xA5A5A5A5` stack canary for corruption detection

The stack memory is initialized with an `0xA5` pattern, and the canary is checked during execution to detect simulated stack overflow or corruption.

## Deadline Monitoring

Each task tracks its release time, relative deadline, absolute deadline, and deadline-miss state.

When a new job is released, its deadline state is reset and a new absolute deadline is calculated. The deadline monitor reports if simulated time reaches that deadline before the job completes.

A separate test scenario intentionally delays a task past its deadline to verify the detection logic.

## Watchdog

The simulation also includes a simple watchdog heartbeat model.

`watchdog_check()` tracks the amount of simulated time since the last known system progress. `watchdog_kick()` resets the counter when the scheduler or a task successfully makes progress.

If the system goes longer than the configured watchdog timeout without progress, a timeout is reported.

## Test Scenarios

Four test scenarios are currently included in `main.c`:

- Scenario 1: normal Rate Monotonic scheduling
- Scenario 2: processor overload
- Scenario 3: deliberate deadline miss
- Scenario 4: priority inversion and priority inheritance

The active scenario is selected with:

```c
int scenario = 4;
```

## Schedulability Analysis

For the main task set:

- Task 1: C = 30 ticks, T = 50 ticks
- Task 2: C = 8 ticks, T = 100 ticks
- Task 3: C = 6 ticks, T = 150 ticks

The total processor utilization is:

```text
U = 30/50 + 8/100 + 6/150
U = 0.72
```

For three periodic tasks, the Liu & Layland sufficient utilization bound is approximately 0.7798.

Since 0.72 is below the bound, the task set passes the Rate Monotonic utilization test.

A more detailed response-time analysis is included in `docs/schedulability_analysis.md`.

## Project Structure

- [`include/kernel.h`](include/kernel.h) - shared kernel types, constants, and function declarations
- [`kernel/kernel.c`](kernel/kernel.c) - scheduler, task management, mutex, deadline monitor, watchdog, and stack checking
- [`kernel/task.c`](kernel/task.c) - simulated task application logic
- [`main.c`](main.c) - simulation harness and test scenarios
- [`docs/design.md`](docs/design.md) - kernel design overview
- [`docs/scheduling_policy.md`](docs/scheduling_policy.md) - scheduling policy and design reasoning
- [`docs/memory_model.md`](docs/memory_model.md) - static memory and stack model
- [`docs/schedulability_analysis.md`](docs/schedulability_analysis.md) - formal schedulability analysis
- [`docs/timing_results.md`](docs/timing_results.md) - simulation timing notes and results

## Build and Run

Requirements:
- Linux or WSL
- GCC with C11 support
- GNU Make

Build the project:

```bash
make
```

Run it:

```bash
make run
```

Clean the build:

```bash
make clean
```

## Skills Covered

- Fixed-priority scheduling
- Rate Monotonic scheduling
- Priority inversion and inheritance
- Task control blocks and task states
- Static memory allocation
- Deadline monitoring
- Watchdog supervision
- Stack integrity checking
- Deterministic task execution
- Liu & Layland utilization analysis
- Worst-case response-time analysis
