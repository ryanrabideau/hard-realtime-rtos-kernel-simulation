# Measured vs Theoretical Timing

## Measured Times (from logs)
- Task 1 critical section: 20–40 ticks (long hold with yields)
- Task 2 long work: 8 ticks per cycle
- Task 3 critical section: 6 ticks
- Switches: negligible (simulation)

## Theoretical (analysis)
- R1 ≈ 30 ticks
- R2 ≈ 38 ticks
- R3 ≈ 44 ticks

## Comparison
- Measured WCET > theoretical due to printf and simulation overhead
- No deadline misses in normal run → under-utilized system
- Watchdog triggered at ~101 ticks (false positive, so no reset on progress)

## Recommendations
- Remove printf in real RT paths
- Reset watchdog on task progress
- Use cycle counters for accurate WCET in embedded port
