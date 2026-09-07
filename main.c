#include <stdio.h>
#include "include/kernel.h"

int main(void) {
    printf("Minimal RTOS preemptive simulation starting...\n");

    kernel_init();

    // Scenario selector:
    // 1 = normal
    // 2 = overload
    // 3 = deadline miss
    // 4 = priority inversion / inheritance
    int scenario = 4;

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
                                   current_task->id,
                                   next->id,
                                   system_ticks);
                        }

                        current_task = next;
                    }

                    current_task->state = TASK_RUNNING;

                    if (system_ticks % 5 == 0) {
                        printf("Running task %u (prio %u) at tick %u\n",
                               current_task->id,
                               current_task->priority,
                               system_ticks);
                    }

                    current_task->entry();

                    if (task_check_stack_overflow(current_task)) {
                        printf("Halting simulation due to stack overflow in task %u\n",
                               current_task->id);
                        break;
                    }

                    if (current_task->state == TASK_RUNNING) {
                        current_task->state = TASK_READY;
                    }
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
                               current_task->id,
                               current_task->priority,
                               system_ticks);
                    }

                    current_task->entry();

                    if (task_check_stack_overflow(current_task)) {
                        printf("Halting due to stack overflow in task %u\n",
                               current_task->id);
                        break;
                    }

                    if (current_task->state == TASK_RUNNING) {
                        current_task->state = TASK_READY;
                    }
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
                               current_task->id,
                               current_task->priority,
                               system_ticks);
                    }

                    // Simulate overrun.
                    for (int extra = 0; extra < 3; extra++) {
                        current_task->entry();
                    }

                    if (task_check_stack_overflow(current_task)) {
                        printf("Halting due to stack overflow in task %u\n",
                               current_task->id);
                        break;
                    }

                    if (current_task->state == TASK_RUNNING) {
                        current_task->state = TASK_READY;
                    }
                }
            }

            break;

        case 4: // Priority inversion / priority inheritance test
            printf("Priority inversion test with inheritance\n");

            /*
             * Reset task application state and keep all tasks inactive so
             * the inversion sequence can be created deliberately.
             */
            for (uint32_t i = 0; i < num_tasks; i++) {
                tasks[i].ready = false;
                tasks[i].state = TASK_READY;
                tasks[i].app_state = 0;
                tasks[i].app_sub_state = 0;
                tasks[i].deadline_miss = false;
            }

            mutex_init(&shared_mutex);

            /*
             * Step 1:
             * Low-priority Task 3 runs first and acquires the mutex.
             */
            kernel_tick();

            current_task = &tasks[2];
            current_task->ready = true;
            current_task->state = TASK_RUNNING;
            current_task->entry();

            if (shared_mutex.owner != &tasks[2]) {
                printf("FAIL: Task 3 did not acquire mutex\n");
                break;
            }

            printf("Low-priority Task 3 owns mutex at priority %u\n",
                   tasks[2].priority);

            /*
             * Task 3 remains ready because it has simulated critical-section
             * work remaining.
             */
            tasks[2].ready = true;
            tasks[2].state = TASK_READY;

            /*
             * Step 2:
             * High-priority Task 1 tries to acquire the same mutex.
             * It should block and cause Task 3 to inherit priority 3.
             */
            kernel_tick();

            current_task = &tasks[0];
            current_task->ready = true;
            current_task->state = TASK_RUNNING;
            current_task->entry();

            printf("After contention: Task 1 state = %d, Task 3 priority = %u\n",
                   tasks[0].state,
                   tasks[2].priority);

            if (tasks[0].state != TASK_BLOCKED) {
                printf("FAIL: Task 1 did not block on mutex\n");
                break;
            }

            if (tasks[2].priority != tasks[0].priority) {
                printf("FAIL: Task 3 did not inherit Task 1 priority\n");
                break;
            }

            /*
             * Step 3:
             * Medium-priority Task 2 becomes ready.
             * Task 3 should continue winning scheduling because it inherited
             * the high priority from Task 1.
             */
            tasks[1].ready = true;
            tasks[1].state = TASK_READY;

            while (shared_mutex.owner == &tasks[2]) {
                kernel_tick();

                tasks[2].ready = true;
                tasks[2].state = TASK_READY;

                tcb_t *next = scheduler_select_next();

                if (next == NULL) {
                    printf("FAIL: no runnable task during inversion test\n");
                    break;
                }

                printf("Scheduler selected Task %u at priority %u\n",
                       next->id,
                       next->priority);

                if (next != &tasks[2]) {
                    printf("FAIL: medium-priority task preempted inherited owner\n");
                    break;
                }

                current_task = next;
                current_task->state = TASK_RUNNING;
                current_task->entry();

                if (task_check_stack_overflow(current_task)) {
                    printf("FAIL: stack overflow detected in Task %u\n",
                           current_task->id);
                    break;
                }

                if (shared_mutex.owner == &tasks[2]) {
                    tasks[2].ready = true;
                    tasks[2].state = TASK_READY;
                }
            }

            /*
             * Step 4:
             * Once Task 3 releases the mutex, its original priority should
             * be restored and Task 1 should be unblocked.
             */
            printf("After release: Task 3 priority = %u, Task 1 state = %d\n",
                   tasks[2].priority,
                   tasks[0].state);

            if (tasks[2].priority != 15) {
                printf("FAIL: Task 3 priority was not restored\n");
                break;
            }

            if (tasks[0].state != TASK_READY || !tasks[0].ready) {
                printf("FAIL: Task 1 was not awakened after mutex release\n");
                break;
            }

            /*
             * Step 5:
             * Scheduler should now select the high-priority Task 1.
             */
            kernel_tick();

            {
                tcb_t *next = scheduler_select_next();

                if (next == NULL) {
                    printf("FAIL: no task selected after mutex release\n");
                    break;
                }

                printf("Scheduler selected Task %u after mutex release\n",
                       next->id);

                if (next != &tasks[0]) {
                    printf("FAIL: Task 1 was not selected after wakeup\n");
                    break;
                }

                current_task = next;
                current_task->state = TASK_RUNNING;
                current_task->entry();
            }

            /*
             * Task 1 should now have acquired the mutex.
             */
            if (shared_mutex.owner == &tasks[0] &&
                tasks[2].priority == 15) {
                printf("PASS: priority inheritance resolved inversion correctly\n");
            } else {
                printf("FAIL: priority inheritance behavior incorrect\n");
            }

            /*
             * Cleanup so the simulation exits with the shared mutex released.
             */
            if (shared_mutex.owner == &tasks[0]) {
                current_task = &tasks[0];
                mutex_unlock(&shared_mutex);

                tasks[0].app_state = 0;
                tasks[0].app_sub_state = 0;
                tasks[0].ready = false;
                tasks[0].state = TASK_READY;
            }

            break;

        default:
            printf("Unknown scenario - running normal mode\n");
            break;
    }

    printf("\n=== Simulation finished after %u ticks ===\n", system_ticks);

    return 0;
}
