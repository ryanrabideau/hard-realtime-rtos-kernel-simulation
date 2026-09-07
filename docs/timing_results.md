# Timing Results

This document summarizes the timing behavior demonstrated by the RTOS simulation.

The kernel uses simulated ticks rather than physical processor timing. The timing values below are therefore part of the deterministic scheduling model and should not be interpreted as measured execution times on real hardware.

## Task Timing Parameters

The main Rate Monotonic task set uses:

- Task 1: period 50 ticks, WCET 30 ticks, deadline 50 ticks
- Task 2: period 100 ticks, WCET 8 ticks, deadline 100 ticks
- Task 3: period 150 ticks, WCET 6 ticks, deadline 150 ticks

This gives a total modeled processor utilization of 72%.

The corresponding worst-case response times from the schedulability analysis are:

- Task 1: 30 ticks
- Task 2: 38 ticks
- Task 3: 44 ticks

All three response times are below their respective deadlines.

See [`schedulability_analysis.md`](schedulability_analysis.md) for the full calculations.

## Normal Scheduling

Scenario 1 runs the three tasks using their configured periods of 50, 100, and 150 ticks.

The scheduler always selects the highest-priority READY task. Since shorter-period tasks have higher priorities, the execution order follows the Rate Monotonic priority assignment.

The normal scenario is intended to demonstrate periodic task releases, fixed-priority selection, deadline monitoring, and deterministic scheduler behavior.

## Overload Test

Scenario 2 deliberately releases Task 1 every tick instead of at its normal 50-tick period.

This creates an artificial overload condition and allows the scheduler behavior to be observed when the highest-priority workload is released much more frequently than intended.

This scenario is a stress test rather than part of the schedulable task model used in the formal analysis.

## Deadline Miss Test

Scenario 3 deliberately releases Task 1 and then prevents it from executing until after its absolute deadline.

The deadline monitor detects the missed deadline and reports the fault before the late task is allowed to execute.

This verifies that deadline monitoring is independent of normal task execution.

## Priority Inheritance Test

Scenario 4 creates a controlled priority-inversion condition.

The sequence is:

1. Low-priority Task 3 acquires the shared mutex.
2. High-priority Task 1 attempts to acquire the mutex and blocks.
3. Task 3 inherits Task 1's higher priority.
4. Medium-priority Task 2 becomes READY.
5. The scheduler continues selecting the boosted Task 3 instead of Task 2.
6. Task 3 finishes its critical section and releases the mutex.
7. Task 3's original priority is restored.
8. Task 1 is awakened and becomes the highest-priority READY task.

The test checks these conditions directly and reports:

```text
PASS: priority inheritance resolved inversion correctly
```

when the expected behavior occurs.

## Watchdog Behavior

The watchdog uses a heartbeat model based on simulated system progress.

`watchdog_check()` advances the watchdog timer once per simulated tick. When a task or the scheduler makes valid progress, `watchdog_kick()` resets the watchdog counter.

If no progress is observed for more than 100 ticks, the watchdog reports a timeout.

This allows the simulation to distinguish an actual lack of progress from normal execution instead of producing a timeout simply because the program has been running for a certain amount of time.

## Stack Monitoring

Each simulated task has a statically allocated 512-byte stack.

The stack is initialized with an `0xA5` pattern and contains an `0xA5A5A5A5` canary. The kernel checks the canary during execution to detect simulated stack corruption.

This is a simplified software model of the stack-integrity checks commonly used in embedded systems.

## Interpretation

The simulation is designed to verify scheduling logic and real-time system concepts rather than benchmark Linux execution performance.

The tick-based results demonstrate:

- Fixed-priority task selection
- Periodic Rate Monotonic releases
- Deadline-miss detection
- Priority inheritance and priority restoration
- Blocked-task wakeup after mutex release
- Watchdog supervision
- Stack integrity monitoring

Physical execution time, interrupt latency, context-switch overhead, and hardware-specific timing are outside the scope of this userspace simulation.
