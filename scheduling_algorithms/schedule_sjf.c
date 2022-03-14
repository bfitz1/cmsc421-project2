#include <stdlib.h>

#include "cpu.h"
#include "schedulers.h"
#include "list.h"
#include "task.h"

struct node* head = NULL;

int compare_burst(Task* t1, Task* t2) {
    return t1->burst < t2->burst;
}

void add(char* name, int priority, int burst) {
    Task* newTask = malloc(sizeof(Task));
    newTask->name = name;
    newTask->priority = priority;
    newTask->burst = burst;

    placeif(&head, newTask, compare_burst);
}

void schedule() {
    while (head) {
        run(head->task, head->task->burst);
        delete(&head, head->task);
    }
}
