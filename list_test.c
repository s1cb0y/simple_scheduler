
#include "list.h"
#include <stdlib.h>
#include <stdio.h>

struct tester_args{
   char *name;
   int iters;
   struct list_head list_entry;
};

int main(){
   struct list_head head;
   list_init(&head);
   struct tester_args *ta1 = malloc(sizeof(*ta1));
   ta1->name = "first";
   ta1->iters = 5;
   list_insert_end(&head, &ta1->list_entry);
   struct tester_args *ta2 = malloc(sizeof(*ta2));
   ta2->name = "second";
   ta2->iters = 3;
   list_insert_end(&head, &ta2->list_entry);

   struct tester_args *var_ptr;

   list_for_each_entry(var_ptr, &head, list_entry, struct tester_args) {
      printf("Task: %s, Iterations: %d\n", var_ptr->name, var_ptr->iters);
   }

   return EXIT_SUCCESS;
}

