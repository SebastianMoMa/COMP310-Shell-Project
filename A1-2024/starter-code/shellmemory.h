//shellmemory.h
// #define MEM_SIZE 10
// #define FRAME_STORE_SIZE 6

// Use macros to set sizes, defined at compile-time using -D flags
#ifndef VARMEMSIZE
#define VARMEMSIZE 10 // Default variable store size (overridden at compile time)
#endif

#ifndef FRAMESIZE
#define FRAMESIZE 6 // Default frame store size (overridden at compile time)
#endif

void mem_init();
char *mem_get_value(int pos);
int mem_check_value(char *var_in);
void mem_set_value(char *var, char *value);


extern int pageTable[4][FRAMESIZE/3]; // can divide by 3 here, since the size will be multiple of 3 

extern int script_count;
extern struct ReadyQueue ready;

struct LineNode {
    char line[100];  
    struct LineNode *next;      
};


struct Script {
    //int id;
    struct LineNode *head;     
    struct LineNode *tail;
    struct LineNode *current;
    int current_instruction_num;     
    int line_count;
    int job_length_score;
    char * name;
    int *frameTable;     // Array of frame indices associated with this script
    int totalPages;      // Total pages (frames) loaded for this script
    int offset;         //Should say the position in the file
    int readyToChange;
};

extern struct Script *scripts[];

struct ReadyQueue {
    struct PCB *head;
    struct PCB *tail;
    int count;
};

struct PCB {
    int pid; //Should correspond to the unique number in array that holds the specific Process (Script)
    struct LineNode *current;
    struct PCB *next;
    int job_length_score;
    int enqueued;
};
extern struct PCB *PCBs[];

struct Frame {
    char lines[3][100]; 
    int pageNumber;     // Page number for this frame
    int processId;      // ID of the process owning the page
    int lastUsed;       // Timestamp for LRU algorithm
    int linesUsed;
    int Started;
            
};
extern struct Frame frameStore[]; // MAX_FRAMES is determined by framesize


// Function prototypes for creating and managing scripts
struct Script* create_script(int id, char* process);
void free_script(struct Script *script);
void add_line_to_script(struct Script *script, const char *line);
void free_pcb(struct PCB *pcb);
void add_to_ready_queue(struct PCB *pcb);
struct PCB* get_next_process();
void clean_up_process(struct PCB *pcb);
void init_scheduler();
void swap_scripts(struct Script **a, struct Script **b);
void sortScriptsByLineCount(int size);
void printScripts(int size);
void printJobLengthScore(int size);
void InfoAboutPCBsandScripts(int size);
void Once_Script_done_AGING();
void Once_Done_AGING();
struct PCB *create_pcb(int pid, struct LineNode *head);
int lookForName (int script_count, char* name);
int findFreeFrame();
void initialize_frame_store();




