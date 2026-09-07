#include "../include/kernel.h"
#include <stdio.h>

static int shared_counter = 0;

void task1_high(void) {
    tcb_t *t = current_task;
    switch (t->app_state) {
        case 0:
            mutex_lock(&shared_mutex);

            if (shared_mutex.owner != t) {
                return;
            }
            
            printf("[T1-high] Tick %u - entered critical section\n", system_ticks);
            t->app_sub_state = 0;
            t->app_state = 1;
            t->ready = false;
            return;
        case 1:
            if (t->app_sub_state < 40) {
                shared_counter++;
                printf("[T1-high] work inside mutex %d - counter %d\n", t->app_sub_state, shared_counter);
                t->app_sub_state++;
                t->ready = false;
                return;
            } else {
                printf("[T1-high] Tick %u - exiting critical section\n", system_ticks);
                mutex_unlock(&shared_mutex);
                t->app_state = 0;
                t->ready = false;
            }
            break;
    }
}

void task2_medium(void) {
    tcb_t *t = current_task;
    switch (t->app_state) {
        case 0:
            printf("[T2-med] Tick %u - starting long work...\n", system_ticks);
            t->app_sub_state = 0;
            t->app_state = 1;
            t->ready = false;
            return;
        case 1:
            if (t->app_sub_state < 8) {
                printf("[T2-med] work step %d\n", t->app_sub_state);
                t->app_sub_state++;
                t->ready = false;
                return;
            } else {
                printf("[T2-med] Tick %u - finished long work\n", system_ticks);
                t->app_state = 0;
                t->ready = false;
            }
            break;
    }
}

void task3_low(void) {
    tcb_t *t = current_task;
    switch (t->app_state) {
        case 0:
            mutex_lock(&shared_mutex);

            if (shared_mutex.owner != t) {
                return;
            }
            
            shared_counter += 10;
            printf("[T3-low] Tick %u - counter now %d\n", system_ticks, shared_counter);
            t->app_sub_state = 0;
            t->app_state = 1;
            t->ready = false;
            return;
        case 1:
            if (t->app_sub_state < 6) {
                printf("[T3-low] holding work step %d\n", t->app_sub_state);
                t->app_sub_state++;
                t->ready = false;
                return;
            } else {
                printf("[T3-low] Tick %u - exiting critical section\n", system_ticks);
                mutex_unlock(&shared_mutex);
                t->app_state = 0;
                t->ready = false;
            }
            break;
    }
}
