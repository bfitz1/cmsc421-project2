/**
 * Various list operations
 */
 
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "list.h"
#include "task.h"


// add a new task to the list of tasks
void insert(struct node **head, Task *newTask) {
    // add the new task to the list 
    struct node *newNode = malloc(sizeof(struct node));

    newNode->task = newTask;
    newNode->next = *head;
    *head = newNode;
}

// add a new task to the end of the list of tasks
void enqueue(struct node **head, Task *newTask) {
    struct node* newNode = malloc(sizeof(struct node));

    newNode->task = newTask;
    newNode->next = NULL;

    if (!(*head)) {
        *head = newNode;
        return;
    }

    struct node* curr = *head;
    while (curr->next) {
        curr = curr->next;
    }
    curr->next = newNode;
}

// add a new task to the list based on its priority
void prioritize(struct node **head, Task *newTask) {
    struct node* newNode = malloc(sizeof(struct node));

    newNode->task = newTask;
    newNode->next = NULL;

    if (!(*head) || (*head)->task->priority < newTask->priority) {
        newNode->next = *head;
        *head = newNode;
        return;
    }

    struct node *curr = *head;
    struct node *prev = NULL;
    while (curr->next && newTask->priority < curr->task->priority) {
        prev = curr;
        curr = curr->next;
    }

    prev->next = newNode;
    newNode->next = curr->next;
}

// delete the selected task from the list
void delete(struct node **head, Task *task) {
    struct node *temp;
    struct node *prev;

    temp = *head;
    // special case - beginning of list
    if (strcmp(task->name,temp->task->name) == 0) {
        *head = (*head)->next;
    }
    else {
        // interior or last element in the list
        prev = *head;
        temp = temp->next;
        while (strcmp(task->name,temp->task->name) != 0) {
            prev = temp;
            temp = temp->next;
        }

        prev->next = temp->next;
    }
}

// traverse the list
void traverse(struct node *head) {
    struct node *temp;
    temp = head;

    while (temp != NULL) {
        printf("[%s] [%d] [%d]\n",temp->task->name, temp->task->priority, temp->task->burst);
        temp = temp->next;
    }
}
