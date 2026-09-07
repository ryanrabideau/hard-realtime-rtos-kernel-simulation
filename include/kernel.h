#ifndef KERNEL_H
#define KERNEL_H

#include <stdint.h>
#include <stdbool.h>

#define TASK_STACK_SIZE 512
#define STACK_CANARY 0xA5A5A5A5
#define WATCHDOG_TIMEOUT_TICKS 100

typedef enum {
    TASK_READY,
    TASK_RUNNING,
    TASK_BLOCKED,
    TASK_SUSPENDED,
    TASK_DEAD
} task_state_t;

typedef void (*task_entry_t)(void);

typedef struct tcb {
    uint32_t          id;
    uint8_t           priority;
    task_state_t      state;
    bool              ready;

    uint32_t          period_ticks;
    uint32_t          deadline_ticks;
    uint32_t          wcet_ticks;

    uint32_t          last_release;
    uint32_t          runtime;

    task_entry_t      entry;

    uint8_t           stack[TASK_STACK_SIZE];
    uint32_t          stack_pointer;
    uint32_t          stack_size_used;

    int               app_state;
    int               app_sub_state;

    uint32_t          release_time;
    uint32_t          deadline_time;
    bool              deadline_miss;
} tcb_t;

typedef struct mutex {
    bool        locked;
    tcb_t      *owner;
    uint8_t     original_prio;
    tcb_t      *blocked_head;
} mutex_t;

extern tcb_t*         current_task;
extern uint32_t       system_ticks;
extern tcb_t          tasks[10];
extern uint32_t       num_tasks;
extern uint32_t       watchdog_counter;

extern mutex_t shared_mutex;

void kernel_init(void);
void kernel_tick(void);

void task_yield(void);
tcb_t* scheduler_select_next(void);

void task_create(uint32_t id,
                 uint8_t priority,
                 task_entry_t entry);

void task_release(uint32_t task_id);

void task_init_stack(tcb_t* t);
bool task_check_stack_overflow(tcb_t* t);

void task_block(tcb_t* t);
void task_unblock(tcb_t* t);

void task_monitor_deadlines(void);

void watchdog_check(void);
void watchdog_kick(void);

void mutex_init(mutex_t* m);
void mutex_lock(mutex_t* m);
void mutex_unlock(mutex_t* m);

extern void task1_high(void);
extern void task2_medium(void);
extern void task3_low(void);

#endif /* KERNEL_H */
