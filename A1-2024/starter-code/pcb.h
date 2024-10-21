//pcb.h

struct PCB;

struct PCB *create_pcb(int pid, struct LineNode *head);

void free_pcb(struct PCB *pcb);