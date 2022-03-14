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
    int slice = QUANTUM;
    while (head) {
        struct node* curr = head;
        while (curr) {
            if (slice == 0) {
                slice = QUANTUM;
            } 
            
            if (slice >= curr->task->burst) {
                run(curr->task, curr->task->burst);
                slice -= curr->task->burst;
                curr->task->burst = 0;

                Task* task = curr->task;
                curr = curr->next;
                delete(&head, task);
            } else {
                run(curr->task, slice);
                curr->task->burst -= slice;
                slice = 0;

                curr = curr->next;
            }
        }
    }
}
