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
struct ReadyQueue ready;

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
    }
    scripts[id % 3] = new_script;
    return new_script;
}

void add_line_to_script(struct Script *script, const char *line){
    struct LineNode *new_line_node = malloc(sizeof(struct LineNode));
    if (new_line_node != NULL && script->line_count <= 100)
    {
        if (new_line_node->line != NULL) {
        strncpy(new_line_node->line, line, sizeof(new_line_node->line));
        new_line_node->next = NULL;

        // Append to the linked list
        if (script->head == NULL)
        {
            script->head = new_line_node;
        }
        else
        {
            script->tail->next = new_line_node;
        }
        script->tail = new_line_node;
        script->line_count++;
        }
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

    //free(script);
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