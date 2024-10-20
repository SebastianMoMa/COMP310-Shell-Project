// pcb.c
#include "pcb.h"
#include <stdlib.h>
#include <stdio.h>
#include "shellmemory.h"

struct PCB {
    int pid; //Should correspond to the unique number in array that holds the specific Process (Script)
    struct LineNode *current;
    struct PCB *next; 
};

// Function to create and initialize a new PCB


struct PCB *create_pcb(int pid, struct LineNode *head)
{
    struct PCB *new_pcb = (struct PCB *)malloc(sizeof(struct PCB));
    if (new_pcb == NULL)
    {
        printf("Error: Memory allocation for PCB failed.\n");
        return NULL;
    }
    new_pcb->pid = pid;
    new_pcb->current = head;
    new_pcb->next = NULL; //Should not be head.next
    PCBs[script_count] = new_pcb;
    script_count++;
    return new_pcb;
}

// Function to free a PCB
void free_pcb(struct PCB *pcb)
{
    if (pcb)
    {
        free(pcb);
    }
}
