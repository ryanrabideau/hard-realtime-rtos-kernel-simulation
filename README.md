# Hard Real-Time Operating System Kernel

A minimal, deterministic, hard real-time operating system kernel written in pure C11, fully simulated in userspace on Linux. Built to master core RTOS concepts: fixed-priority preemptive scheduling, priority-inheritance mutexes, static memory management, deadline monitoring, watchdog fault detection, and formal schedulability analysis, all without dynamic allocation, threads, or external dependencies.

## Project Overview

This project implements a tiny hard RTOS kernel that runs deterministically in a single-threaded simulation loop. Key goals:
- Demonstrate Rate Monotonic Scheduling with preemption
- Prevent priority inversion via inheritance in mutexes
- Enforce static memory (no malloc/free in RT paths)
- Detect timing faults (deadline misses, stalls)
- Provide formal analysis (Liu & Layland bound, response times, measured vs theoretical)

No hardware, drivers, or real-time OS APIs. Everything is self-contained and testable on any Linux desktop.

## Features

- Fixed-priority preemptive scheduler (Rate Monotonic)
- Mutexes with priority inheritance (blocks high-prio tasks from inversion)
- Static task array + per-task 512-byte stacks with canary-based overflow detection
- Tick-driven time model with simulated context (state machines + yield returns)
- Deadline monitoring & logging of misses
- Watchdog for stall/timeout detection
- Deterministic execution (no randomness, no pthreads)
- Formal schedulability analysis (utilization bound + response-time equations)

## Project Structure

- docs/ — Design docs and formal analysis (manifesto.md, design.md, scheduling_policy.md, memory_model.md, schedulability_analysis.md, timing_results.md)
- include/ — Shared headers (kernel.h)
- kernel/ — Core kernel files (kernel.c, task.c)
- main.c — Simulation harness + test scenarios

## Prerequisites & Build

**Requirements**  
- Linux (or WSL on Windows)
- GCC (C11 support)
- make

**Build and execute:**

- make
- make run

**Clean build artifacts:**

- make clean

Change int scenario = 1; in main.c to test different modes (overload, deadline miss, etc.).

## Skills Covered

- Real-time systems design: Rate Monotonic scheduling, priority inheritance, response-time analysis
- Deterministic programming: static allocation, no dynamic memory in RT paths
- Fault tolerance: deadline monitoring, watchdog, overflow detection
- Kernel fundamentals: task control blocks, simulated preemption, state machines
- Formal analysis: Liu & Layland utilization bound, worst-case response times
- Embedded-style C: no OS dependencies, minimal runtime, predictable behavior
- Documentation: design decisions, lessons learned, timing comparison
