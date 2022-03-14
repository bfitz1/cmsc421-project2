/**
 * list data structure containing the tasks in the system
 */

#include "task.h"

typedef int (*compare) (Task* a, Task* b);

struct node {
    Task *task;
    struct node *next;
};

// insert and delete operations.
void insert(struct node **head, Task *task);
void enqueue(struct node **head, Task *task);
void placeif(struct node **head, Task *task, compare cmp);
void delete(struct node **head, Task *task);
void traverse(struct node *head);
