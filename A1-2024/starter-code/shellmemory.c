//shellmemory.c
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include "shellmemory.h"

// Memory structure holds key-value pairs in shell memory
struct memory_struct
{
    char *var;
    char *value;
};

// Global arrays for managing shellmemory, framestore
struct memory_struct shellmemory[VARMEMSIZE]; // Shell memory storage
struct Frame frameStore[FRAMESIZE];           // Frame store for page management
int pageTable[4][FRAMESIZE / 3];              // Page table for memory mapping

// Arrays to manage scripts and processes
struct Script *scripts[10]; // Store script references (max of 10)
int script_count = 0;       // Track number of scripts for use when updating scripts

struct PCB *PCBs[10];
// Ready queue for process scheduling
struct ReadyQueue ready = {NULL, NULL, 0}; // Ready queue initialization

// Initializing the frame store with default (-1) values
void initialize_frame_store() {
    for (int i = 0; i < FRAMESIZE; i++) {
        // Initialize lines to empty strings
        for (int j = 0; j < 3; j++) {
            frameStore[i].lines[j][0] = '\0';  // Set each line to an empty string
        }

        // Initialize pageNumber, processId, and lastUsed to default values
        frameStore[i].pageNumber = -1;   // -1 means no page assigned
        frameStore[i].processId = -1;    // -1 means no process assigned
        frameStore[i].lastUsed = -1;     // -1 means not used yet
        frameStore[i].linesUsed = 0;     // No lines used
        frameStore[i].Started = 0;       // Frame not started
    }
}

// Find the first available free frame in the frame store
// Returns the index of a free frame, or -1 if no free frame is found
int findFreeFrame(){
    for (int i = 0; i < FRAMESIZE/3; i++){
        // A frame is considered free if it is not assigned to any process
        if (frameStore[i].processId == -1){
            return i; //  The first found index is that of the first free frame
        }
    }
    return -1; // no free index was found, return -1
}

// Create a new script with given ID and name
// Allocates memory and initializes script metadata
struct Script *create_script(int id, char *name){
    struct Script *new_script = malloc(sizeof(struct Script));
    if (new_script != NULL)
    {
        // Initial script data
        new_script->head = NULL;
        new_script->tail = NULL;
        new_script->line_count = 0;
        new_script->current = new_script->head;
        new_script->current_instruction_num=0;
        scripts[id % 10] = new_script;
        new_script->job_length_score=0;
        new_script->name = name;
        new_script->offset = 0;
        new_script->totalPages = 0;
        new_script->readyToChange=0;
    }

    else {
        return NULL;
    }
    return new_script;
}

//  Look for a script with a specific name in the scripts array
// Made for case where two processes have same name
int lookForName (int script_count, char* name){
    int index = -1;
    for (int i = 0; i < script_count; i++){
        //Update index when name has been seen before;
        if (strcmp(scripts[i]->name, name)==0){
            index = i;
        }
    }
    return index;
}

// Add a line to the script's linked list of lines
void add_line_to_script(struct Script *script, const char *line){
    struct LineNode *new_line_node = malloc(sizeof(struct LineNode));
    if (new_line_node != NULL && script->line_count <= 100)
    {
        // Copy line content and set up the new line node
        strncpy(new_line_node->line, line, sizeof(new_line_node->line));
        new_line_node->next = NULL;

        // Append to the linked list
        if (script->head == NULL || script->current ==NULL)
        {
            script->head = new_line_node;
            script->current=script->head;
        }
        else
        {
            script->tail->next = new_line_node;
        }

        // Update script data
        script->tail = new_line_node;
        script->line_count++;
        script->job_length_score++;
        
    }
}

// Free all memory related to a script
void free_script(struct Script *script){
    if (script == NULL)
        return;

    // Traverse and free all line nodes
    struct LineNode *current = script->head;
    struct LineNode *next_node;

    while (current != NULL)
    {
        next_node = current->next;
        free(current);
        current = next_node;
    }

    // Reset script metadata
    script->head = NULL;
    script->tail = NULL;
    script->line_count = 0;

    free(script);
}

// This function creates a new Process Control Block (PCB)
// Initializes process metadata then adds it to the global PCBs array
struct PCB *create_pcb(int pid, struct LineNode *head)
{ 
    struct PCB *new_pcb = (struct PCB *)malloc(sizeof(struct PCB));
    if (new_pcb == NULL)
    {
        printf("Error: Memory allocation for PCB failed.\n");
        return NULL;
    }
    // Initialize PCB data
    new_pcb->pid = pid;
    new_pcb->current = head;
    new_pcb->next = NULL; //Should not be head.next
    PCBs[script_count % 10] = new_pcb; // Store PCB in global PCBs array
    new_pcb->job_length_score=0;
    script_count++;
    new_pcb->enqueued = 0;
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

// Add a PCB to the ready queue
void add_to_ready_queue(struct PCB *pcb) {
    pcb->next = NULL;
    // If the queue is not empty, append to the tail
    if (ready.tail) {
        ready.tail->next = pcb;
    } else {
        // If the queue is empty, set as head
        ready.head = pcb;
    }
    // Update the ready tail and increment the count
    ready.tail = pcb;
    ready.count++;
}

// Get the next process from the ready queue
struct PCB* get_next_process() {
    if (ready.head == NULL) return NULL;

    // Remove head from the queue
    struct PCB *pcb = ready.head;
    ready.head = ready.head->next;
    pcb = ready.head;

    // If queue becomes empty, reset tail to null
    if (ready.head == NULL) {
        ready.tail = NULL;
    }
    ready.count--;
    return pcb;
}

// Implement aging mechanism for process scheduling
void Once_Done_AGING() {
    struct PCB *old_head = ready.head;

    //First we decrease the job_length score
    //One Script
    if (old_head->next == NULL) {
        ready.head = NULL;
        ready.tail = NULL;
        free(old_head); // Clean up the old head
        return;
    }

    // Adjust job length scores based on number of scripts (2)
    if (script_count==2){
        if (old_head->next->job_length_score > 0)
        {
            old_head->next->job_length_score--;
            scripts[old_head->next->pid]->job_length_score--;
        }
    }
    else if (script_count ==3){
        // If 3 scripts, decrease job length scores for middle and tail processes
        if (old_head->next->job_length_score > 0)
        {
            old_head->next->job_length_score--; // Decrease middle job_length_score
            scripts[old_head->next->pid]->job_length_score--;
        }
        if (ready.tail->job_length_score > 0)
        {
            ready.tail->job_length_score--;
            scripts[ready.tail->pid]->job_length_score--;
        }
    }

    //Remove head from Queue and clean up
    ready.head = ready.head->next;
    script_count --;
    if (ready.head == NULL) {
        ready.tail = NULL;
    }
    clean_up_process(old_head);

    //Restructure the Queue, for lowest job_length_score at head
    if (script_count == 2){ // This case means that it was three and just got decreased
        int compare = ready.head->job_length_score - ready.tail->job_length_score; // Should be same if you put ready.next instead of ready.tail
        if (compare > 0){
            //If tail has lower score, than put it at head
            ready.head = ready.tail;
            ready.head->next = old_head->next;
            ready.tail = ready.head->next;
            ready.tail->next = NULL;
        }
    }
    // We do not have to worry if script_count == 1
}

// Clean up a process by freeing its script and PCB
void clean_up_process(struct PCB *pcb) {
    // Free the script associated with the process and the PCB itself
    if (pcb) {
        free_script(scripts[pcb->pid]);
        free_pcb(pcb);
    }
}

// Initialize the scheduler by resetting the ready queue
void init_scheduler() {
    ready.head = NULL;
    ready.tail = NULL;
    ready.count = 0;
}

// Swap two script references
void swap_scripts(struct Script **a, struct Script **b) {
    struct Script *temp = *a;
    *a = *b;
    *b = temp;

    
}

// Swap two PCB references
void swap_PCBS(struct PCB **a, struct PCB **b) {
    struct PCB *temp = *a;
    *a = *b;
    *b = temp;
}

// Swap PIDs between two PCBs
void swap_pids(struct PCB *pcb1, struct PCB *pcb2) {
    int temp_pid = pcb1->pid;
    pcb1->pid = pcb2->pid;
    pcb2->pid = temp_pid;
}

// Sort scripts by line count using bubble sort
void sortScriptsByLineCount(int size) {
    for (int i = 0; i < size - 1; i++) {
        for (int j = 0; j < size - i - 1; j++) {
            if (scripts[j]->line_count > scripts[j + 1]->line_count) {
                swap_scripts(&scripts[j], &scripts[j + 1]);
            }
        }
    }
}

// Print information about scripts, useful for debugging
void printScripts(int size) {
    for (int i = 0; i < size; i++) {
        printf("Script %d, Line Count: %d\n", i, scripts[i]->line_count);
        printf("pid: %d, PCB line_count: %d\n", PCBs[i]->pid,scripts[PCBs[i]->pid]->line_count);
    }
}

// Print job length scores for scripts, for debugging
void printJobLengthScore(int size){
for (int i = 0; i < size; i++) {
        printf("Script %d, job_length_score: %d\n", i, scripts[i]->job_length_score);
    }
}

// Print detailed information about PCBs and Scripts
void InfoAboutPCBsandScripts(int size) {
    for (int i = 0; i < size; i++) {
        printf("Script %d, Line Count: %d PCB pid: %d, Script Line Count: %d\n", i, scripts[i]->line_count, PCBs[i]->pid, scripts[PCBs[i]->pid]->line_count);
    }
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
// Initialize shell memory with default "none" values
void mem_init()
{
    int i;
    for (i = 0; i < VARMEMSIZE; i++)
    {
        shellmemory[i].var = "none";
        shellmemory[i].value = "none";
    }
}

// Set key value pair
void mem_set_value(char *var_in, char *value_in)
{
    int i;
    for (i = 0; i < VARMEMSIZE; i++)
    {
       // printf("bool for strcmp(shellmemory[i].var, var_in): %d, shellmemory[i].var: %s, var_in: %s\n", strcmp(shellmemory[i].var, var_in),shellmemory[i].var, var_in);
        if (strcmp(shellmemory[i].var, var_in) == 0)
        {
           // printf("Here13\n");
            shellmemory[i].value = strdup(value_in);
            return;
        }
        
    }

    // Value does not exist, need to find a free spot.
    for (i = 0; i < VARMEMSIZE; i++)
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
    for (i = 0; i < VARMEMSIZE; i++)
    {
        if (strcmp(shellmemory[i].var, var_in) == 0)
        {
            return i;
        }
    }
    return -1;
}

// Get the value of a variable from shell memory
char *mem_get_value(int j)
{
    // if non-neggative value, value exists -> return it. Otherwise print message.
    return (j >= 0) ? strdup(shellmemory[j].value) : "Variable does not exist";
}