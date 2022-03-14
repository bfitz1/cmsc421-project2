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
        if (!(head->next) || compare_priority(head->task, head->next->task)) {
            run(head->task, head->task->burst);
            delete(&head, head->task);
        } else {
            // Split into a sublist of equal priority tasks
            int priority = head->task->priority;
            struct node* curr = head;
            while (curr->next && curr->next->task->priority == priority) {
                curr = curr->next;
            }
            struct node* sublist = head;
            head = curr->next;
            curr->next = NULL;

            // Process the sublist round-robin style
            int slice = QUANTUM;
            while (sublist) {
                struct node* curr = sublist;
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
                        delete(&sublist, task);
                    } else {
                        run(curr->task, slice);
                        curr->task->burst -= slice;
                        slice = 0;

                        curr = curr->next;
                    }
                }
            }
        }
    }
}
