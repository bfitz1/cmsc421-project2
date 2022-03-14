#include <stdlib.h>

#include "cpu.h"
#include "schedulers.h"
#include "list.h"
#include "task.h"

struct node* head = NULL;

void add(char* name, int priority, int burst) {
    Task* newTask = malloc(sizeof(Task));
    newTask->name = name;
    newTask->priority = priority;
    newTask->burst = burst;

    enqueue(&head, newTask);
}

void schedule() {
    while (head) {
        run(head->task, head->task->burst);
        delete(&head, head->task);
    }
}
