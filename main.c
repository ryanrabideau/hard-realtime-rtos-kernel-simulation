#include <stdio.h>
#include "include/kernel.h"

static void run_selected_task(tcb_t *next) {
    if (next == NULL) {
        return;
    }

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

    current_task->entry();

    /*
     * Successfully executing a task step counts as observable
     * system progress for the watchdog model.
     */
    watchdog_kick();

    if (task_check_stack_overflow(current_task)) {
        printf("Halting simulation due to stack overflow in Task %u\n",
               current_task->id);
        return;
    }

    /*
     * Preserve legitimate BLOCKED / SUSPENDED / DEAD states.
     */
    if (current_task->state == TASK_RUNNING) {
        current_task->state = TASK_READY;
    }
}

int main(void) {
    printf("Minimal RTOS preemptive simulation starting...\n");

    kernel_init();

    /*
     * Scenario selector:
     * 1 = normal Rate Monotonic scheduling
     * 2 = overload
     * 3 = deadline miss
     * 4 = priority inversion / inheritance
     */
    int scenario = 4;

    printf("Running scenario %d\n", scenario);

    switch (scenario) {

        case 1:
            printf("Normal Rate Monotonic scheduling test\n");

            /*
             * Start with all tasks inactive. Their first jobs
             * are explicitly released at tick 1.
             */
            for (uint32_t i = 0; i < num_tasks; i++) {
                tasks[i].ready = false;
                tasks[i].state = TASK_READY;
                tasks[i].app_state = 0;
                tasks[i].app_sub_state = 0;
                tasks[i].deadline_miss = false;
                tasks[i].deadline_time = 0;
            }

            for (uint32_t tick = 1; tick <= 500; tick++) {
                kernel_tick();

                /*
                 * Periodic releases using the canonical task set:
                 *
                 * Task 1: period 50
                 * Task 2: period 100
                 * Task 3: period 150
                 */
                if ((tick - 1) % tasks[0].period_ticks == 0) {
                    task_release(1);
                }

                if ((tick - 1) % tasks[1].period_ticks == 0) {
                    task_release(2);
                }

                if ((tick - 1) % tasks[2].period_ticks == 0) {
                    task_release(3);
                }

                task_monitor_deadlines();
                watchdog_check();

                tcb_t *next = scheduler_select_next();

                if (next != NULL) {
                    printf("Running Task %u (prio %u) at tick %u\n",
                           next->id,
                           next->priority,
                           system_ticks);

                    run_selected_task(next);
                } else {
                    /*
                     * Idle operation is also legitimate forward progress
                     * in this simulation. The scheduler is responsive and
                     * there is simply no READY task.
                     */
                    watchdog_kick();

                    if (system_ticks % 25 == 0) {
                        printf("Idle at tick %u\n",
                               system_ticks);
                    }
                }
            }

            break;

        case 2:
            printf("Overload test: Task 1 released every tick\n");

            for (uint32_t i = 0; i < num_tasks; i++) {
                tasks[i].ready = false;
                tasks[i].state = TASK_READY;
                tasks[i].app_state = 0;
                tasks[i].app_sub_state = 0;
                tasks[i].deadline_miss = false;
                tasks[i].deadline_time = 0;
            }

            for (uint32_t tick = 1; tick <= 200; tick++) {
                kernel_tick();

                /*
                 * Deliberately violate Task 1's intended period by
                 * releasing it continuously.
                 */
                task_release(1);

                if ((tick - 1) % tasks[1].period_ticks == 0) {
                    task_release(2);
                }

                if ((tick - 1) % tasks[2].period_ticks == 0) {
                    task_release(3);
                }

                task_monitor_deadlines();
                watchdog_check();

                tcb_t *next = scheduler_select_next();

                if (next != NULL) {
                    printf("Running Task %u (prio %u) at tick %u\n",
                           next->id,
                           next->priority,
                           system_ticks);

                    run_selected_task(next);
                } else {
                    watchdog_kick();
                }
            }

            break;

        case 3:
            printf("Deadline miss test\n");

            /*
             * Keep Tasks 2 and 3 inactive so the experiment focuses
             * specifically on Task 1.
             */
            for (uint32_t i = 0; i < num_tasks; i++) {
                tasks[i].ready = false;
                tasks[i].state = TASK_READY;
                tasks[i].app_state = 0;
                tasks[i].app_sub_state = 0;
                tasks[i].deadline_miss = false;
                tasks[i].deadline_time = 0;
            }

            /*
             * Release one Task 1 job at tick 1.
             */
            kernel_tick();
            task_release(1);

            /*
             * Artificially prevent Task 1 from executing until
             * after its absolute deadline.
             */
            tasks[0].ready = false;

            for (uint32_t tick = 2;
                 tick <= tasks[0].deadline_ticks + 5;
                 tick++) {

                kernel_tick();
                task_monitor_deadlines();
                watchdog_check();

                /*
                 * The harness itself remains alive while the task
                 * is deliberately withheld, so service the simulated
                 * watchdog.
                 */
                watchdog_kick();
            }

            /*
             * Allow the late job to execute after the miss has
             * already been detected.
             */
            tasks[0].ready = true;
            tasks[0].state = TASK_READY;

            tcb_t *next = scheduler_select_next();

            if (next != NULL) {
                printf("Executing late Task %u at tick %u\n",
                       next->id,
                       system_ticks);

                run_selected_task(next);
            }

            break;

        case 4:
            printf("Priority inversion test with inheritance\n");

            /*
             * Reset task application state and keep all tasks inactive
             * so the inversion sequence can be created deliberately.
             */
            for (uint32_t i = 0; i < num_tasks; i++) {
                tasks[i].ready = false;
                tasks[i].state = TASK_READY;
                tasks[i].app_state = 0;
                tasks[i].app_sub_state = 0;
                tasks[i].deadline_miss = false;
                tasks[i].deadline_time = 0;
            }

            mutex_init(&shared_mutex);

            /*
             * Step 1:
             * Low-priority Task 3 acquires the shared mutex.
             */
            kernel_tick();

            current_task = &tasks[2];
            current_task->ready = true;
            current_task->state = TASK_RUNNING;
            current_task->entry();

            watchdog_kick();

            if (shared_mutex.owner != &tasks[2]) {
                printf("FAIL: Task 3 did not acquire mutex\n");
                break;
            }

            printf("Low-priority Task 3 owns mutex at priority %u\n",
                   tasks[2].priority);

            tasks[2].ready = true;
            tasks[2].state = TASK_READY;

            /*
             * Step 2:
             * High-priority Task 1 tries to acquire the same mutex.
             * It should block and boost Task 3.
             */
            kernel_tick();

            current_task = &tasks[0];
            current_task->ready = true;
            current_task->state = TASK_RUNNING;
            current_task->entry();

            watchdog_kick();

            printf("After contention: Task 1 state = %d, "
                   "Task 3 priority = %u\n",
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
             * Medium-priority Task 2 becomes READY.
             * The inherited Task 3 priority should keep Task 2
             * from preempting the mutex owner.
             */
            tasks[1].ready = true;
            tasks[1].state = TASK_READY;

            while (shared_mutex.owner == &tasks[2]) {
                kernel_tick();
                watchdog_check();

                tasks[2].ready = true;
                tasks[2].state = TASK_READY;

                tcb_t *next = scheduler_select_next();

                if (next == NULL) {
                    printf("FAIL: no runnable task "
                           "during inversion test\n");
                    break;
                }

                printf("Scheduler selected Task %u "
                       "at priority %u\n",
                       next->id,
                       next->priority);

                if (next != &tasks[2]) {
                    printf("FAIL: medium-priority task "
                           "preempted inherited owner\n");
                    break;
                }

                current_task = next;
                current_task->state = TASK_RUNNING;
                current_task->entry();

                watchdog_kick();

                if (task_check_stack_overflow(current_task)) {
                    printf("FAIL: stack overflow detected "
                           "in Task %u\n",
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
             * Task 3 should have released the mutex,
             * restored its original priority, and awakened Task 1.
             */
            printf("After release: Task 3 priority = %u, "
                   "Task 1 state = %d\n",
                   tasks[2].priority,
                   tasks[0].state);

            if (tasks[2].priority != 15) {
                printf("FAIL: Task 3 priority was not restored\n");
                break;
            }

            if (tasks[0].state != TASK_READY ||
                !tasks[0].ready) {

                printf("FAIL: Task 1 was not awakened "
                       "after mutex release\n");

                break;
            }

            /*
             * Step 5:
             * Task 1 should now be the highest-priority READY task.
             */
            kernel_tick();
            watchdog_check();

            {
                tcb_t *next =
                    scheduler_select_next();

                if (next == NULL) {
                    printf("FAIL: no task selected "
                           "after mutex release\n");
                    break;
                }

                printf("Scheduler selected Task %u "
                       "after mutex release\n",
                       next->id);

                if (next != &tasks[0]) {
                    printf("FAIL: Task 1 was not selected "
                           "after wakeup\n");
                    break;
                }

                current_task = next;
                current_task->state = TASK_RUNNING;
                current_task->entry();

                watchdog_kick();
            }

            if (shared_mutex.owner == &tasks[0] &&
                tasks[2].priority == 15) {

                printf("PASS: priority inheritance "
                       "resolved inversion correctly\n");

            } else {

                printf("FAIL: priority inheritance "
                       "behavior incorrect\n");
            }

            /*
             * Cleanup.
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
            printf("Unknown scenario\n");
            break;
    }

    printf("\n=== Simulation finished after %u ticks ===\n",
           system_ticks);

    return 0;
}
