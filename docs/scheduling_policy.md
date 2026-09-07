# Scheduling Policy

The kernel uses fixed-priority preemptive scheduling based on Rate Monotonic principles.

## Priority Assignment

Each task is assigned a static priority. Lower numeric values represent higher scheduling priorities.

The main task set uses:

- Task 1: priority 3, period 50 ticks
- Task 2: priority 8, period 100 ticks
- Task 3: priority 15, period 150 ticks

Since shorter-period tasks receive higher priorities, the ordering follows Rate Monotonic scheduling.

## Scheduler Behavior

The scheduler searches the task array for the highest-priority task that is both READY and marked as runnable.

When a higher-priority task becomes READY, it is selected before any lower-priority READY task at the next scheduling decision.

Tasks are implemented as deterministic state machines rather than independent operating-system threads. Each task invocation performs a bounded step of work and then returns control to the simulation.

Because this project runs in a single-threaded userspace environment, preemption is simulated at scheduler decision points rather than performed through hardware interrupts or CPU context switching.

## Priority Inversion

A lower-priority task can temporarily block a higher-priority task if it owns a mutex required by the higher-priority task.

The kernel handles this using priority inheritance.

When a higher-priority task blocks on the mutex:

1. The waiting task enters the BLOCKED state.
2. The mutex owner temporarily inherits the waiting task's priority.
3. The boosted owner can run ahead of intermediate-priority tasks.
4. When the mutex is released, the owner's original priority is restored.
5. The blocked task is awakened and becomes eligible to run again.

Scenario 4 in [`../main.c`](../main.c) creates this condition deliberately and verifies the expected priority-inheritance behavior.

## Schedulability

The modeled task set has a total processor utilization of 72%.

For three periodic tasks, the Liu & Layland sufficient Rate Monotonic utilization bound is approximately 77.98%. Since the modeled utilization is below this bound, the task set passes the sufficient utilization test.

Worst-case response-time analysis also shows that all three tasks complete within their modeled deadlines.

The complete calculations are available in [`schedulability_analysis.md`](schedulability_analysis.md).

## Scope

The scheduler is intended to demonstrate real-time scheduling concepts in a deterministic simulation.

It does not implement hardware context switching, interrupt-driven preemption, multicore scheduling, or dynamic priority policies. Those features are outside the scope of this project.
