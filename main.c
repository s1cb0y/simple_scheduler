#include <stdio.h>
#include <stdlib.h>
#include "scheduler.h"

struct tester_args{
    char *name;
    int iters;
};

void tester(void *args){
    int i;
    struct tester_args *ta = (struct tester_args *) args;
    for (i = 0; i < ta->iters; i++){
        printf("task %s: %d \n", ta->name, i);
        scheduler_relinquish();
    }
    free(ta);
}

void create_task(char* name, int iters){
    printf("create task : %s\n", name);
    struct tester_args *ta = malloc(sizeof(*ta));
    ta->name = name;
    ta->iters = iters;
    scheduler_create_task(tester, ta);
}

int main(){
    scheduler_init();
    create_task("first", 5);

    create_task("second", 3);
    scheduler_run();
    printf("Finished running all tasks \n");
    return EXIT_SUCCESS;
}

