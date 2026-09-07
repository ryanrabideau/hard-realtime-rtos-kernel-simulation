# Project Goals

This project was built to better understand what happens inside a real-time operating system instead of only learning how to use an existing RTOS API.

The main goal was to implement the core concepts in a small C codebase where the scheduling behavior could be followed directly.

## Design Goals

The project focuses on:

- Deterministic fixed-priority scheduling
- Rate Monotonic priority assignment
- Priority inversion and priority inheritance
- Static memory allocation
- Deadline monitoring
- Watchdog fault detection
- Stack integrity checking
- Formal schedulability analysis

The implementation avoids operating-system threads and dynamic memory allocation so the behavior remains simple and repeatable.

## Keep the System Deterministic

The simulation uses a single-threaded execution model and simulated system ticks.

Tasks are implemented as state machines that perform bounded steps of work before returning control to the scheduler.

This makes it possible to reproduce scheduling scenarios without depending on Linux thread timing or host operating-system behavior.

## Make Failure Cases Testable

The project is not limited to normal scheduler operation.

The test harness also includes scenarios for:

- Processor overload
- Deadline misses
- Priority inversion
- Priority inheritance

These cases make it possible to observe how the kernel responds when normal timing or resource-sharing assumptions are violated.

## Keep Memory Bounded

Task control blocks and simulated stacks are statically allocated.

The kernel does not use dynamic memory allocation, which keeps the memory requirements bounded and avoids heap-related behavior during execution.

## Connect Implementation to Analysis

The scheduler is paired with formal Rate Monotonic schedulability analysis.

The modeled task set is checked using both the Liu & Layland utilization bound and worst-case response-time analysis.

The calculations are documented in [`schedulability_analysis.md`](schedulability_analysis.md).

## Scope

This project is a deterministic userspace simulation of RTOS concepts, not a production RTOS.

It intentionally does not implement hardware context switching, interrupt-driven scheduling, memory protection, or multicore execution.

The purpose is to build and test the fundamental scheduling, synchronization, timing, and memory concepts in a codebase small enough to understand from end to end.
