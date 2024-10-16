#ifndef SCRIPT_H
#define SCRIPT_H

#define MAX_USER_INPUT 1024  // Adjust 

struct Script scripts[3];
int script_count = 0;

struct LineNode {
    char line[MAX_USER_INPUT];  
    struct LineNode *next;      
};


struct Script {
    int id;
    struct LineNode *head;     
    struct LineNode *tail;
    struct LineNode current;
    int current_instruction_num;     
    int line_count;            
};

struct ScriptList {
    struct Script scripts[3]; // This array should account for their possibly being up to 3 scripts
    int script_count;                    // Count of scripts in the list
} Scripty;


struct ReadyQueue {
    struct PCB *head;
    struct PCB *tail;
    int count;
} ready;

struct PCB {
    int pid; //Should correspond to the unique number in array that holds the specific Process (Script)
    int current_instruction;
    struct PCB *next; 
};

// Function prototypes for creating and managing scripts
struct Script* create_script(const char *name);
void free_script(struct Script *script);
void add_line_to_script(struct Script *script, const char *line);


#endif 