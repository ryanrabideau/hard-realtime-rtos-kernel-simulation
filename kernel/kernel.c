#include "../include/kernel.h"
#include <stdio.h>
#include <stddef.h>
#include <string.h>

tcb_t*      current_task = NULL;
uint32_t    system_ticks = 0;
tcb_t       tasks[10] = {0};
uint32_t    num_tasks = 0;
uint32_t    watchdog_counter = 0;

mutex_t shared_mutex = {0};

void kernel_init(void) {
    printf("Kernel initializing...\n");

    mutex_init(&shared_mutex);

    task_create(1, 3, task1_high);
    task_create(2, 8, task2_medium);
    task_create(3, 15, task3_low);

    current_task = &tasks[0];
    printf("Kernel init complete. First task: %u\n", current_task->id);
}

void kernel_tick(void) {
    system_ticks++;
}

void task_yield(void) {
    printf("Task %u yields at tick %u\n", current_task->id, system_ticks);
}

tcb_t* scheduler_select_next(void) {
    tcb_t *best = NULL;
    uint8_t highest_prio = 255;

    for (uint32_t i = 0; i < num_tasks; i++) {
        tcb_t *t = &tasks[i];
        if (t->ready && t->state == TASK_READY && t->priority < highest_prio) {
            highest_prio = t->priority;
            best = t;
        }
    }

    return best;
}

void task_create(uint32_t id, uint8_t priority, task_entry_t entry) {
    if (num_tasks >= 10) {
        printf("Error: Too many tasks!\n");
        return;
    }

    tcb_t *t = &tasks[num_tasks];
    t->id = id;
    t->priority = priority;
    t->state = TASK_READY;
    t->ready = true;
    t->entry = entry;
    t->period_ticks = 0;
    t->deadline_ticks = 50;
    t->wcet_ticks = 10;
    t->last_release = 0;
    t->runtime = 0;
    t->release_time = 0;
    t->deadline_time = 0;
    t->deadline_miss = false;
    t->app_state = 0;
    t->app_sub_state = 0;

    task_init_stack(t);

    num_tasks++;
    printf("Task %u created (prio %u, deadline %u ticks, WCET %u ticks)\n",
           id, priority, t->deadline_ticks, t->wcet_ticks);
}

void task_release(uint32_t task_id) {
    for (uint32_t i = 0; i < num_tasks; i++) {
        if (tasks[i].id == task_id) {
            tasks[i].state = TASK_READY;
            tasks[i].ready = true;
            tasks[i].last_release = system_ticks;
            tasks[i].release_time = system_ticks;
            if (tasks[i].deadline_ticks > 0) {
                tasks[i].deadline_time = system_ticks + tasks[i].deadline_ticks;
                printf("Task %u released at %u, deadline at %u\n", task_id, system_ticks, tasks[i].deadline_time);
            }
            return;
        }
    }
    printf("Task %u not found for release!\n", task_id);
}

void task_init_stack(tcb_t* t) {
    memset(t->stack, 0xA5, TASK_STACK_SIZE);
    t->stack_pointer = TASK_STACK_SIZE - sizeof(uint32_t);
    t->stack_size_used = 0;
    *(uint32_t*)(t->stack) = STACK_CANARY;
    printf("Task %u stack initialized (%u bytes, canary set)\n", t->id, TASK_STACK_SIZE);
}

bool task_check_stack_overflow(tcb_t* t) {
    if (*(uint32_t*)(t->stack) != STACK_CANARY) {
        printf("!!! STACK OVERFLOW DETECTED in Task %u !!!\n", t->id);
        return true;
    }
    return false;
}

void task_monitor_deadlines(void) {
    for (uint32_t i = 0; i < num_tasks; i++) {
        tcb_t *t = &tasks[i];
        if (t->deadline_time == 0) continue;
        if (system_ticks >= t->deadline_time && !t->deadline_miss) {
            t->deadline_miss = true;
            printf("!!! DEADLINE MISS in Task %u !!! (current %u, deadline %u)\n",
                   t->id, system_ticks, t->deadline_time);
        }
    }
}

void watchdog_check(void) {
    watchdog_counter++;
    if (watchdog_counter > 100) {
        printf("!!! WATCHDOG TIMEOUT !!! No progress in %u ticks\n", watchdog_counter);
        watchdog_counter = 0;
    }
}

void mutex_init(mutex_t* m) {
    m->locked = false;
    m->owner = NULL;
    m->original_prio = 0;
    m->blocked_head = NULL;
    printf("Mutex initialized\n");
}

void task_block(tcb_t* t) {
    t->state = TASK_BLOCKED;
    t->ready = false;
    printf("Task %u blocked\n", t->id);
}

void task_unblock(tcb_t* t) {
    t->state = TASK_READY;
    t->ready = true;
    printf("Task %u unblocked\n", t->id);
}

void mutex_lock(mutex_t* m) {
    if (!m->locked) {
        m->locked = true;
        m->owner = current_task;
        m->original_prio = current_task->priority;
        printf("Task %u locks mutex\n", current_task->id);
    } else if (m->owner == current_task) {
        printf("Error: Task %u already owns mutex!\n", current_task->id);
    } else {
        task_block(current_task);

        m->blocked_head = current_task;

        printf("Task %u waits on mutex held by %u\n",
               current_task->id, m->owner->id);

        if (current_task->priority < m->owner->priority) {
            printf("Inheritance: boosting owner %u from prio %u to %u\n",
                   m->owner->id,
                   m->owner->priority,
                   current_task->priority);

            m->owner->priority = current_task->priority;
        }

        task_yield();
    }
}

void mutex_unlock(mutex_t* m) {
    if (m->owner != current_task) {
        printf("Error: Task %u doesn't own mutex!\n", current_task->id);
        return;
    }

    tcb_t *waiter = m->blocked_head;

    if (current_task->priority != m->original_prio) {
        printf("Restoring owner prio from %u to %u\n",
               current_task->priority,
               m->original_prio);

        current_task->priority = m->original_prio;
    }

    m->locked = false;
    m->owner = NULL;
    m->blocked_head = NULL;

    printf("Task %u unlocks mutex\n", current_task->id);

    if (waiter != NULL) {
        task_unblock(waiter);
        printf("Task %u awakened after mutex release\n", waiter->id);
    }
}
