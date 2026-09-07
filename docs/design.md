# Kernel Design

## Overview

The project implements a small deterministic RTOS kernel simulation in C11. It runs as a single-threaded userspace program on Linux and models the behavior of a fixed-priority real-time scheduler.

The simulation focuses on scheduling, task state management, mutex behavior, deadline monitoring, watchdog supervision, and static memory rather than hardware-specific context switching.

## Task Model

Each task is represented by a task control block containing:

- Task ID
- Static priority
- Current task state
- Runnable status
- Period
- Relative deadline
- WCET parameter
- Release and deadline timing information
- Application state
- Simulated stack
- Stack monitoring information

Tasks are implemented as state machines. Each invocation performs a bounded step of work and then returns control to the simulation.

The kernel supports the following task states:

- `TASK_READY`
- `TASK_RUNNING`
- `TASK_BLOCKED`
- `TASK_SUSPENDED`
- `TASK_DEAD`

## Scheduler

The scheduler uses fixed priorities.

At each scheduling decision, it searches the statically allocated task array and selects the highest-priority task that is READY and runnable.

Lower numeric priority values represent higher scheduling priority.

The main task set follows Rate Monotonic priority ordering:

- Task 1: priority 3, period 50 ticks
- Task 2: priority 8, period 100 ticks
- Task 3: priority 15, period 150 ticks

Because the simulation is single-threaded, task preemption is modeled at scheduler decision points rather than through hardware interrupts or CPU context switching.

More detail is available in [`scheduling_policy.md`](scheduling_policy.md).

## Task Releases and Deadlines

Periodic jobs are activated through `task_release()`.

A release:

- Marks the task READY
- Records its release time
- Calculates its new absolute deadline
- Clears the deadline-miss state from the previous job

`task_monitor_deadlines()` checks the active task deadlines against the current simulated system tick.

If a deadline is reached before the job completes, the kernel reports a deadline miss. Deadline monitoring is used for fault detection; it does not automatically reset the system or terminate the task.

## Mutex and Priority Inheritance

The kernel includes a shared mutex used to demonstrate priority inversion and priority inheritance.

If the mutex is free, the requesting task becomes its owner.

If another task already owns the mutex, the requesting task becomes BLOCKED. If the blocked task has a higher priority than the owner, the owner temporarily inherits that higher priority.

When the owner releases the mutex:

- Its original priority is restored
- The mutex becomes available
- The blocked waiter is moved back to the READY state

The awakened task must run again before it can acquire the mutex.

The current implementation intentionally models a single blocked waiter instead of a full RTOS wait queue.

## Static Memory

Task control blocks and task stacks are statically allocated.

The kernel supports up to 10 tasks, with a 512-byte simulated stack reserved for each task.

No dynamic memory allocation is used by the kernel.

Each task stack is initialized with an `0xA5` pattern and contains an `0xA5A5A5A5` canary used to detect simulated stack corruption.

More detail is available in [`memory_model.md`](memory_model.md).

## Watchdog

The watchdog provides a simple heartbeat-based stall detector.

`watchdog_check()` advances the watchdog counter as simulated time passes. `watchdog_kick()` resets the counter when valid system progress occurs.

If the watchdog exceeds its configured timeout without being serviced, the kernel reports a timeout.

The watchdog is a fault-detection mechanism in this simulation. It does not automatically reset the program.

## Simulation Scenarios

The test harness in [`../main.c`](../main.c) includes four scenarios:

- Normal Rate Monotonic scheduling
- Processor overload
- Deliberate deadline miss
- Priority inversion with priority inheritance

These scenarios make it possible to exercise normal scheduling behavior as well as several common real-time failure conditions.

## Design Scope

This project is intended to demonstrate the internal concepts behind a real-time kernel in a small and inspectable codebase.

It models:

- Fixed-priority scheduling
- Rate Monotonic priority assignment
- Task state transitions
- Priority inheritance
- Static memory allocation
- Deadline monitoring
- Watchdog supervision
- Stack integrity checking

It does not implement:

- Hardware context switching
- Interrupt-driven scheduling
- Memory protection
- Multiple CPU cores
- Dynamic task creation
- A full mutex waiter queue

Keeping those features outside the project scope allows the scheduling and fault-handling behavior to remain deterministic and easy to test.
