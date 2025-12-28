#pragma once
#include <stdlib.h>
#include <stdio.h>
#include <setjmp.h>
#include "list.h"
#include <stdint.h>
struct task{
    enum {
        ST_CREATED,
        ST_RUNNING,
        ST_WAITING,
    } status;

    int id;

    jmp_buf buf;

    void (*func)(void*);
    void *arg;

    struct list_head task_list;

    void *stack_bottom;
    void *stack_top;
    int stack_size;
};

struct scheduler_private{
    jmp_buf buf;
    struct task *current;
    struct list_head task_list;
} priv;

void scheduler_init(void){
    priv.current = NULL;
    list_init(&priv.task_list);
}

void scheduler_create_task(void (*func)(void*), void* arg){
    static int id = 1;
    struct task *task = malloc(sizeof(*task));
    task->status = ST_CREATED;
    task->func = func;
    task->arg = arg;
    task->id = id++;
    task->stack_size = 16 * 1024;
    task->stack_bottom = malloc(task->stack_size);
    if (!task->stack_bottom) {
        perror("malloc");
        exit(1);
    }
    /* compute top using byte arithmetic and reserve 16 bytes for alignment/safety */
    task->stack_top = (void *)((char *)task->stack_bottom + task->stack_size - 16);
    list_insert_end(&priv.task_list, &task->task_list);
}

enum {
    INIT=0,
    SCHEDULE,
    EXIT_TASK,
};


static struct task *scheduler_choose_task(void){
    struct task *task;
    list_for_each_entry(task, &priv.task_list, task_list, struct task)
    {
        if (task->status == ST_RUNNING || task->status == ST_CREATED) {
            list_remove(&task->task_list);
            list_insert_end(&priv.task_list, &task->task_list);
            return task;
        }
    }

    return NULL;
}

// is called by the task directly.
// freeing resources of current task here directly
// would lead to using an already freed stack
void scheduler_exit_current_task(void){
    struct task *task = priv.current;
    list_remove(&task->task_list);
    longjmp(priv.buf, EXIT_TASK);
}

void scheduler_free_current_task(void){
    struct task *task = priv.current;
    priv.current = NULL;
    free(task->stack_bottom);
    free(task);
}

static void schedule(void){
    struct task *next = scheduler_choose_task();

    if (!next){
        return; //nothing to do anymore
    }

    priv.current = next;
    if (next->status == ST_CREATED){
        // assign a new stack pointer, run and exit at the end
#if defined(__aarch64__)
        uintptr_t top_val = (uintptr_t)next->stack_top;
        __asm__ volatile(
            "mov sp, %x0\n"
            :
            : "r"(top_val)
            : "memory"
        );
    #elif defined(__x86_64__) || defined(__amd64__)
        register void *top = next->stack_top;
        asm volatile(
            "mov %[rs], %%rsp \n"
            : [ rs ] "+r" (top) ::
        );
    #else
    #error "Unsupported architecture for setting stack pointer"
    #endif

       // run the task function. Use `priv.current` after switching SP
        priv.current->status = ST_RUNNING;
        {
            void (*task_fn)(void *) = priv.current->func;
            void *task_arg = priv.current->arg;
            task_fn(task_arg);
        }
        // stack pointer should be back where we set it, so just returning would be
        // a bad idea -> exit the task correctly
        scheduler_exit_current_task();
    } else {
        longjmp(next->buf, 1); // resume running task, longjmp restores SP of task automatically
    }
}

void scheduler_run(void){
    switch (setjmp(priv.buf)){
        case EXIT_TASK:
            scheduler_free_current_task();
            // fallthrough
        case INIT:
        case SCHEDULE:
            schedule(); // if this returns, all tasks are done
            return;
        default:
            // should not happen
            fprintf(stderr, "scheduler bug\n");
            return;
    }
}
void scheduler_relinquish(void){
    // setjmp saves the context (SP, ...) of the running task
    // so no need for inline assembly here
    if (setjmp(priv.current->buf)){
        return;
    } else {
        longjmp(priv.buf, SCHEDULE);
    }
}
