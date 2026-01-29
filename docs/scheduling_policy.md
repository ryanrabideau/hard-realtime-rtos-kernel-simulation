# Scheduling Policy Choice

## Chosen Policy: Fixed-Priority Preemptive (Rate Monotonic)
Shorter period = higher priority.

## Why Rate Monotonic?
- Simpler implementation and analysis than EDF
- Industry standard for hard real-time systems
- Predictable, as it is easy to prove schedulability
- Sufficient utilization for this project (~70% bound for n=3)

## How It Works
- Priorities assigned statically (lower number = higher priority)
- Scheduler always selects highest-priority READY task
- Preemption occurs when a higher-prio task becomes READY

## Tradeoffs
Pros: Low overhead, analyzable (Liu & Layland bound), deterministic  
Cons: Utilization capped (~69–78% depending on n), not optimal for aperiodic tasks

Chosen over EDF for first implementation. Easier to verify and debug.
