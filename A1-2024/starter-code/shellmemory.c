#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include "shellmemory.h"
// #include "script.h"

struct memory_struct
{
    char *var;
    char *value;
};

struct memory_struct shellmemory[MEM_SIZE];

struct Script *scripts[3];
int script_count = 0;
//struct ReadyQueue ready;
struct PCB *PCBs[3];
struct ReadyQueue ready = {NULL, NULL, 0}; // Ready queue initialization



struct Script *create_script(int id){
    struct Script *new_script = malloc(sizeof(struct Script));
    if (new_script != NULL)
    {
        new_script->id = id;
        // strncpy(new_script->name, name, sizeof(new_script->name));
        new_script->head = NULL;
        new_script->tail = NULL;
        new_script->line_count = 0;
        new_script->current = new_script->head;
        new_script->current_instruction_num=0;
        scripts[id % 3] = new_script;
    }
    else {
        return NULL;
    }
    return new_script;
}

void add_line_to_script(struct Script *script, const char *line){
    struct LineNode *new_line_node = malloc(sizeof(struct LineNode));
    if (new_line_node != NULL && script->line_count <= 100)
    {
        strncpy(new_line_node->line, line, sizeof(new_line_node->line));
        new_line_node->next = NULL;

        // Append to the linked list
        if (script->head == NULL)
        {
            script->head = new_line_node;
            script->current=script->head;
        }
        else
        {
            script->tail->next = new_line_node;
        }
        script->tail = new_line_node;
        script->line_count++;
        
    }
}

void free_script(struct Script *script){
    if (script == NULL)
        return;

    struct LineNode *current = script->head;
    struct LineNode *next_node;

    while (current != NULL)
    {
        next_node = current->next;
        free(current);
        current = next_node;
    }

    script->head = NULL;
    script->tail = NULL;
    script->line_count = 0;

    free(script);
}


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

void add_to_ready_queue(struct PCB *pcb) {
    pcb->next = NULL;
    if (ready.tail) {
        ready.tail->next = pcb;
    } else {
        ready.head = pcb;
    }
    ready.tail = pcb;
    ready.count++;
}

struct PCB* get_next_process() {
    //printf("this is ready.head %s\n", ready.head->current->line);
    if (ready.head == NULL) return NULL;
    struct PCB *pcb = ready.head;
    ready.head = ready.head->next;
    pcb = ready.head;
    if (ready.head == NULL) {
        ready.tail = NULL;
    }
    ready.count--;
    return pcb;
}

void clean_up_process(struct PCB *pcb) {
    // Free the script associated with the process and the PCB itself
    if (pcb) {
        free_script(scripts[pcb->pid]);
        free_pcb(pcb);
    }
}

void init_scheduler() {
    ready.head = NULL;
    ready.tail = NULL;
    ready.count = 0;
}



// Helper functions
int match(char *model, char *var)
{
    int i, len = strlen(var), matchCount = 0;
    for (i = 0; i < len; i++)
    {
        if (model[i] == var[i])
            matchCount++;
    }
    if (matchCount == len)
    {
        return 1;
    }
    else
        return 0;
}

// Shell memory functions

void mem_init()
{
    int i;
    for (i = 0; i < MEM_SIZE; i++)
    {
        shellmemory[i].var = "none";
        shellmemory[i].value = "none";
    }
}

// Set key value pair
void mem_set_value(char *var_in, char *value_in)
{
    int i;

    for (i = 0; i < MEM_SIZE; i++)
    {
        if (strcmp(shellmemory[i].var, var_in) == 0)
        {
            shellmemory[i].value = strdup(value_in);
            return;
        }
    }

    // Value does not exist, need to find a free spot.
    for (i = 0; i < MEM_SIZE; i++)
    {
        if (strcmp(shellmemory[i].var, "none") == 0)
        {
            shellmemory[i].var = strdup(var_in);
            shellmemory[i].value = strdup(value_in);
            return;
        }
    }
    return;
}

// helper function to check if var exists in memory
int mem_check_value(char *var_in)
{
    int i;
    for (i = 0; i < MEM_SIZE; i++)
    {
        if (strcmp(shellmemory[i].var, var_in) == 0)
        {
            return i;
        }
    }
    return -1;
}

char *mem_get_value(int j)
{
    // if non-neggative value, value exists -> return it. Otherwise print message.
    return (j >= 0) ? strdup(shellmemory[j].value) : "Variable does not exist";
}