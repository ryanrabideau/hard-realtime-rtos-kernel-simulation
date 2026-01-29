#include <stdio.h>
#include "include/kernel.h"

int main(void) {
    printf("Minimal RTOS preemptive simulation starting...\n");

    kernel_init();

    // Scenario selector (change this number to run different tests)
    int scenario = 1;  // 1 = normal, 2 = overload, 3 = deadline miss, 4 = inversion

    printf("Running scenario %d\n", scenario);

    switch (scenario) {
        case 1: // Normal run - 200 ticks
            for (uint32_t tick = 0; tick < 200; tick++) {
                kernel_tick();
                task_monitor_deadlines();
                watchdog_check();

                if (tick % 3 == 0) {
                    task_release(1);
                    task_release(2);
                    task_release(3);
                }

                tcb_t *next = scheduler_select_next();

                if (next != NULL) {
                    if (current_task != next) {
                        if (current_task != NULL) {
                            printf("Switching from %u to %u at tick %u\n",
                                   current_task->id, next->id, system_ticks);
                        }
                        current_task = next;
                    }

                    current_task->state = TASK_RUNNING;

                    if (system_ticks % 5 == 0) {
                        printf("Running task %u (prio %u) at tick %u\n",
                               current_task->id, current_task->priority, system_ticks);
                    }

                    current_task->entry();

                    if (task_check_stack_overflow(current_task)) {
                        printf("Halting simulation due to stack overflow in task %u\n", current_task->id);
                        break;
                    }

                    current_task->state = TASK_READY;
                } else {
                    if (system_ticks % 10 == 0) {
                        printf("Idle at tick %u\n", system_ticks);
                    }
                }
            }
            break;

        case 2: // Overload: Task 1 dominates
            printf("Overload test: Task 1 runs continuously\n");
            for (uint32_t tick = 0; tick < 200; tick++) {
                kernel_tick();
                task_monitor_deadlines();
                watchdog_check();
                tasks[0].ready = true;
                tasks[0].state = TASK_READY;
                tcb_t *next = scheduler_select_next();
                if (next != NULL) {
                    current_task = next;
                    current_task->state = TASK_RUNNING;
                    if (system_ticks % 5 == 0) {
                        printf("Running task %u (prio %u) at tick %u\n",
                               current_task->id, current_task->priority, system_ticks);
                    }
                    current_task->entry();
                    if (task_check_stack_overflow(current_task)) {
                        printf("Halting due to stack overflow in task %u\n", current_task->id);
                        break;
                    }
                    current_task->state = TASK_READY;
                }
            }
            break;

        case 3: // Deadline miss test
            printf("Deadline miss test: Task 1 overruns\n");
            for (uint32_t tick = 0; tick < 200; tick++) {
                kernel_tick();
                task_monitor_deadlines();
                watchdog_check();
                if (tick % 3 == 0) {
                    task_release(1);
                    task_release(2);
                    task_release(3);
                }
                tcb_t *next = scheduler_select_next();
                if (next != NULL && next->id == 1) {
                    current_task = next;
                    current_task->state = TASK_RUNNING;
                    if (system_ticks % 5 == 0) {
                        printf("Running task %u (prio %u) at tick %u\n",
                               current_task->id, current_task->priority, system_ticks);
                    }
                    // Simulate overrun
                    for (int extra = 0; extra < 3; extra++) {
                        current_task->entry();
                    }
                    if (task_check_stack_overflow(current_task)) {
                        printf("Halting due to stack overflow in task %u\n", current_task->id);
                        break;
                    }
                    current_task->state = TASK_READY;
                }
            }
            break;

        default:
            printf("Unknown scenario - running normal mode\n");
    }

    printf("\n=== Simulation finished after %u ticks ===\n", system_ticks);
    return 0;
}
