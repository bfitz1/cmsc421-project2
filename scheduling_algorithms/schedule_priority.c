#include <stdlib.h>

#include "cpu.h"
#include "schedulers.h"
#include "list.h"
#include "task.h"

struct node* head = NULL;

int compare_priority(Task* t1, Task* t2) {
    return t1->priority > t2->priority;
}

void add(char* name, int priority, int burst) {
    Task* newTask = malloc(sizeof(Task));
    newTask->name = name;
    newTask->priority = priority;
    newTask->burst = burst;

    placeif(&head, newTask, compare_priority);
}

void schedule() {
    while (head) {
        run(head->task, head->task->burst);
        delete(&head, head->task);
    }
}
