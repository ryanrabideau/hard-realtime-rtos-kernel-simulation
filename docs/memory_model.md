# Static Memory Model

## Overview

The kernel uses static memory allocation to keep memory usage deterministic.

No `malloc()`, `calloc()`, `realloc()`, or `free()` calls are used by the kernel. Task control blocks and simulated task stacks are allocated before runtime through fixed-size global storage.

## Task Storage

The kernel maintains a statically allocated array that supports up to 10 tasks.

```c
tcb_t tasks[10];
```

Each task control block stores the information needed by the scheduler and simulation, including:

- Task ID and priority
- Task state and runnable status
- Period, deadline, and WCET parameters
- Release and deadline timing information
- Task entry function
- Application state
- Simulated stack information

This provides a fixed upper bound on the memory used for task management.

## Task Stacks

Each task control block contains a statically allocated 512-byte simulated stack.

```c
uint8_t stack[TASK_STACK_SIZE];
```

with:

```c
#define TASK_STACK_SIZE 512
```

The stack is part of the task control block itself, so creating a task does not require heap allocation.

These stacks model static stack reservation and integrity checking. They are not used for real CPU context switching because the project runs as a single-threaded userspace simulation.

## Stack Initialization

When a task is created, its stack is filled with the byte pattern:

```text
0xA5
```

This provides a recognizable initialization pattern commonly used when inspecting stack memory.

A 32-bit canary is also placed at the bottom of the simulated stack:

```c
#define STACK_CANARY 0xA5A5A5A5
```

The kernel checks this value during execution. If the canary has changed, the simulation reports a stack overflow or corruption condition.

## Deterministic Memory Usage

The main statically allocated task storage consists of:

- Up to 10 task control blocks
- One 512-byte simulated stack per task
- Fixed kernel state
- A statically allocated shared mutex

There are no runtime heap allocations or dynamically resized task structures.

This means the kernel's memory requirements are bounded before the simulation begins.

## Scope

The memory model is intended to demonstrate static-allocation practices used in embedded and real-time systems.

It does not implement:

- A heap allocator
- Dynamic task creation beyond the fixed task array
- Memory protection
- Virtual memory
- Hardware stack switching
- MPU or MMU support

The goal is to keep memory behavior simple, bounded, and deterministic while demonstrating the design principles behind static memory use in real-time software.
