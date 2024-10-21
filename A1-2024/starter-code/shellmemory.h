//shellmemory.h
#define MEM_SIZE 1000
void mem_init();
char *mem_get_value(int pos);
int mem_check_value(char *var_in);
void mem_set_value(char *var, char *value);


extern int script_count;
extern struct ReadyQueue ready;

struct LineNode {
    char line[100];  
    struct LineNode *next;      
};


struct Script {
    int id;
    struct LineNode *head;     
    struct LineNode *tail;
    struct LineNode *current;
    int current_instruction_num;     
    int line_count;            
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
};
extern struct PCB *PCBs[];

// Function prototypes for creating and managing scripts
struct Script* create_script(int id);
void free_script(struct Script *script);
void add_line_to_script(struct Script *script, const char *line);
void free_pcb(struct PCB *pcb);
void add_to_ready_queue(struct PCB *pcb);
struct PCB* get_next_process();
void clean_up_process(struct PCB *pcb);
void init_scheduler();
