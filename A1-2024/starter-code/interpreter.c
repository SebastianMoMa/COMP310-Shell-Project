// interpreter.c
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <sys/stat.h>
#include <unistd.h>
#include "shellmemory.h"
#include "shell.h"
#include "dirent.h"
#include "ctype.h"
#include "pcb.h"

// #include "script.h"

int MAX_ARGS_SIZE = 7;

int badcommand()
{
    printf("Unknown Command\n");
    return 1;
}

int badcommandTooManyTokens()
{
    printf("Bad command: Too many tokens\n");
    return 2;
}

// For run command only
int badcommandFileDoesNotExist()
{
    printf("Bad command: File not found\n");
    return 3;
}

int badcommandMkDirectory()
{
    printf("Bad command: my_mkdir\n");
    return 3;
}

int badcommandMyCd()
{
    printf("Bad command: my_cd\n");
    return 3;
}

int badcommandtooManyProcesses()
{
    printf("Bad command: too many Processes for exec");
    return 4;
}

int help();
int quit();
int set(char *var, char *value);
int print(char *var);
int run(char *script);
int echo(char *value);
int my_ls();
int my_mkdir(char *dirname);
int my_cd(char *dirname);
int my_touch(char *filename);
bool isOne(char *word);
bool isAlphaNum(char *word);
int badcommandFileDoesNotExist();
int exec(char *processes[], int numProcesses, char *policy);
void FCFS();
void load_queue_FCFS();

// Interpret commands and their arguments
int interpreter(char *command_args[], int args_size)
{
    int i;
    // printf("args_size 1: %d\n", args_size);

    if (args_size < 1)
    {
        return badcommand();
    }
    else if (args_size > MAX_ARGS_SIZE)
    {
        return badcommandTooManyTokens();
    }

    for (i = 0; i < args_size; i++)
    { // terminate args at newlines
        command_args[i][strcspn(command_args[i], "\r\n")] = 0;
    }

    if (strcmp(command_args[0], "help") == 0)
    {
        // help
        if (args_size != 1)
            return badcommand();
        return help();
    }
    else if (strcmp(command_args[0], "quit") == 0)
    {
        // quit
        if (args_size != 1)
            return badcommand();
        return quit();
    }
    // set
    else if (strcmp(command_args[0], "set") == 0)
    {
        if (args_size < 3)
        {
            return badcommand();
        }
        // else if	(args_size>7) return badcommandTooManyTokens();
        char value[MAX_USER_INPUT] = "";
        for (int i = 2; i < args_size; i++)
        {
            if (i > 2)
                strcat(value, " "); // Add space if not the first token
            strcat(value, command_args[i]);
        }
        return set(command_args[1], value);
    }
    // print
    else if (strcmp(command_args[0], "print") == 0)
    {
        if (args_size != 2)
            return badcommand();
        return print(command_args[1]);
    }
    // run
    else if (strcmp(command_args[0], "run") == 0)
    {
        if (args_size != 2)
            return badcommand();
        // printf("Going into run\n");
        return run(command_args[1]);
    }

    else if (strcmp(command_args[0], "echo") == 0)
    {
        if (args_size != 2)
            return badcommand();
        return echo(command_args[1]);
    }

    else if (strcmp(command_args[0], "my_ls") == 0)
    {
        return my_ls();
    }

    else if (strcmp(command_args[0], "my_mkdir") == 0)
    {
        if (args_size != 2)
            return badcommand();
        return my_mkdir(command_args[1]);
    }

    else if (strcmp(command_args[0], "my_touch") == 0)
    {
        if (args_size != 2)
            return badcommand();
        return my_touch(command_args[1]);
    }

    else if (strcmp(command_args[0], "my_cd") == 0)
    {
        if (args_size != 2)
            return badcommand();
        return my_cd(command_args[1]);
    }
    else if (strcmp(command_args[0], "exec") == 0)
    {
        if (args_size > 5)
        {
            return badcommandtooManyProcesses();
        }
        char *processes[args_size - 2];
        int numProcesses = 0;
        for (int i = 1; i < args_size - 1; i++)
        {
            processes[i - 1] = command_args[i];
            numProcesses++;
        }
        return exec(processes, numProcesses, command_args[args_size - 1]);
    }

    else
        return badcommand();
}

int help()
{

    // note the literal tab characters here for alignment
    char help_string[] = "COMMAND			DESCRIPTION\n \
help			Displays all the commands\n \
quit			Exits / terminates the shell with “Bye!”\n \
set VAR STRING		Assigns a value to shell memory\n \
print VAR		Displays the STRING assigned to VAR\n \
run SCRIPT.TXT		Executes the file SCRIPT.TXT\n ";
    printf("%s\n", help_string);
    return 0;
}

int quit()
{
    printf("Bye!\n");
    exit(0);
}

int set(char *var, char *value)
{
    // printf("var: %s, value: %s\n", var, value);
    mem_set_value(var, value);
    return 0;
}

int echo(char *arg)
{
    if (arg[0] == '$')
    {
        char *var_name = arg + 1;
        // gets the position of the variable in memory if it exists
        int pos = mem_check_value(var_name);
        if (pos >= 0)
        {
            printf("%s\n", mem_get_value(pos)); // Get and print the value if it exists
        }
        else
        {
            printf("\n"); // Print a blank line if the variable is not found
        }
    }
    else
    {
        printf("%s\n", arg);
    }
    return 0;
}

int compare(const struct dirent **a, const struct dirent **b)
{
    if (isdigit((**a).d_name[0]) && isdigit((*b)->d_name[0]))
    {
        /*
        if ((**a).d_name[0] > (*b)->d_name[0]) {
            return -1;
        }
        else if ((**a).d_name[0] < (*b)->d_name[0]) {
            return 1;
        }
        else {return 0;}
        */
        return strcmp((*a)->d_name, (*b)->d_name); // I'm pretty sure this function should just work
    }
    else if (isdigit((*a)->d_name[0]))
    {
        return -1;
    }

    else if (isdigit((*b)->d_name[0]))
    {
        return 1;
    }
    else
    {
        return strcmp((**a).d_name, (*b)->d_name);
    }
}

int my_ls()
{
    struct dirent **nameList;
    int n = scandir(".", &nameList, NULL, compare);

    for (int i = 0; i < n; i++)
    {
        if (nameList[i]->d_name[0] == '.')
        {
            continue;
        }
        printf("%s\n", nameList[i]->d_name);
        free(nameList[i]);
    }
    free(nameList);

    return 0;
}

bool isAlphaNum(char *word)
{
    int i = 0;
    while (word[i] != '\0')
    {
        if (!(isalnum(word[i])))
            return false;
        i++;
    }
    return true;
}

bool isOne(char *word)
{
    int i = 0;
    while (word[i] != '\0')
    {
        if (isspace(word[i]))
            return false;
        i++;
    }
    return true;
}

int my_mkdir(char *dirname)
{
    if (dirname[0] == '$')
    {
        char *var_name = dirname + 1;
        // gets the position of the variable in memory if it exists
        int pos = mem_check_value(var_name);
        if (pos >= 0)
        {
            // make directory with the value it got
            // check if only one value
            char *value = mem_get_value(pos);
            if (isOne(value))
            {
                mkdir(value, 0755);
            }
            else
            {
                return badcommandMkDirectory();
            }
        }
        else
        {
            return badcommandMkDirectory();
        }
    }
    else
    {
        if (isAlphaNum(dirname))
        {
            mkdir(dirname, 0755);
        }
    }
    return 0;
}

int my_touch(char *filename)
{
    FILE *fptr;
    fptr = fopen(filename, "w");
    fclose(fptr);
}

int my_cd(char *dirname)
{
    struct stat sbuf;
    if (stat(dirname, &sbuf) != 0)
        return badcommandMyCd();
    chdir(dirname);
}

int print(char *var)
{
    int i = mem_check_value(var);
    printf("%s\n", mem_get_value(i));
    return 0;
}

int loadProcessestoMemory(char *process)
{
    //printf("Inside loadProcessestoMemory\n");
    //printf("loading script: %s\n", process);
    int errCode = 0;
    char line[MAX_USER_INPUT];
    FILE *p = fopen(process, "rt");

    if (p == NULL)
    {
        // f//printf(stderr, "Failed to open script file: %s\n", process);
        return badcommandFileDoesNotExist();
    }

    struct Script *new_script = create_script(script_count); // Initialize the script
    if (new_script == NULL)
    {
        fclose(p);
        // f//printf(stderr, "Failed to create new script\n");
        return errCode;
    }
    //printf("Created new script with count: %d\n", script_count);

    while (1)//(fgets(line, sizeof(line), p) !=NULL)
    {
        
    
        if (fgets(line, sizeof(line), p) ==NULL) {
            add_line_to_script(new_script, " ");
            printf("This is line where fgets(line, sizeof(line), p) ==NULL: %s\n", line);
            break;
        }
        if (feof(p))
        {
            //printf("This is last line: %s\n", line);
            add_line_to_script(new_script, line);            
            break;
        }
        //printf("This is line: %s\n", line);
        add_line_to_script(new_script, line);
        //printf("Added line to script: %s\n", line);
    }
    struct PCB *script_pcb = create_pcb(script_count, new_script->current);
    if (script_pcb == NULL)
    {
        fclose(p);
        // f//printf(stderr, "Failed to create PCB for script\n");
        return errCode;
    }
    //printf("script_count: %d\n", script_count);

    //printf("Created PCB with id: %d\n", script_pcb->pid);
    add_to_ready_queue(script_pcb);
    struct PCB *for_pcb = ready.head;
    for (int i =0; i < script_count; i++){
        //printf("This is pcb %d in ready queue: %s\n", i, for_pcb->current->line);
        for_pcb = for_pcb->next;
    }
    //printf("leaving loadProcessestoMemory\n");

    fclose(p);
}

int exec(char *processes[], int numProcesses, char *policy)
{
    //printf("Inside exec\n");

    int errcode = 0;
    //printf("This is numProcesses: %d\n", numProcesses);
    //printf("This is the policy: '%s'\n", policy);
    for (int i = 0; i < numProcesses; i++)
    {
        //printf("This is process %d: %s\n", i, processes[i]);
    }

    //if (!(strcmp(policy, "FCFS") == 0 || strcmp(policy, "SJF") == 0 || strcmp(policy, "RR") == 0 || strcmp(policy, "AGING") == 0))
    if (!(strcmp(policy, "f") == 0 || strcmp(policy, "s") == 0 || strcmp(policy, "f") == 0 || strcmp(policy, "a") == 0))

    {
        //printf("Error: Invalid Policy\n");
        return 4;
    }

    if (numProcesses == 1)
    {
        //printf("Loading single process: %s\n", processes[0]);
        loadProcessestoMemory(processes[0]);
        load_queue_FCFS();
        //printf("Executing FCFS for single process.\n");
        FCFS();
        return 3;
    }
    else
    {
        //printf("Loading multiple processes...\n");
        for (int i = 0; i < numProcesses; i++)
        {
            loadProcessestoMemory(processes[i]);
        }
        if (strcmp(policy, "f") == 0)
        {
            load_queue_FCFS();
            //printf("Executing FCFS for multiple processes.\n");
            FCFS();
        }
        else if (strcmp(policy, "SJF") == 0)
        {
            //printf("Have not fully implemented %s yet... sorry :)\n", policy);
        }
        else if (strcmp(policy, "RR") == 0)
        {
            //printf("Have not fully implemented %s yet... sorry :)\n", policy);
        }
        else if (strcmp(policy, "AGING") == 0)
        {
            //printf("Have not fully implemented %s yet... sorry :)\n", policy);
        }
        //printf("leaving exec\n");

        return errcode;
    }
}

void load_queue_FCFS()
{
    //printf("inside load_queue_FCFS\n");

    struct PCB *script_pcb = NULL;
    //printf("this is script_count: %d\n", script_count);

    for (int i = 0; i < script_count; i++)
    {
        script_pcb = PCBs[i];
        //printf("this is PCB %s 1\n", scripts[script_pcb->pid]->head->line);
        add_to_ready_queue(script_pcb);
        //printf("Added PCB with id: %d to the ready queue.\n", script_pcb->pid);
    }
    //printf("leaving load_queue_FCFS\n");
}

void FCFS2()
{
}

void FCFS()
{
    //printf("Starting FCFS function\n");
    int while_num = 0;
    struct PCB *current_process = ready.head;

    while (ready.head != NULL)
    {
        
        //printf("Inside FCFS while_loop. while_num = %d\n", while_num);
        while_num++;

        // Check if current_process is valid
        if (current_process != NULL)
        {
            //printf("Current process id: %d\n", current_process->pid);
        }
        else
        {
            //printf("No current process found.\n");
            break; // Prevent dereferencing NULL
        }

        struct Script *current_script = scripts[current_process->pid]; // Get the process from array
        //printf("Current script head line: %s\n", current_script->head->line);
        struct LineNode *current_line_node = current_script->current;
        int instruction_num = current_script->current_instruction_num; // Local variable for instruction tracking
        //printf("Current script line_num: %d\n", instruction_num);

        int while_num2 = 0;
        //printf("Current script line count: %d\n", current_script->line_count);


        while (instruction_num < current_script->line_count && current_line_node != NULL)
        {
            //printf("Inside FCFS inner while loop. while_num2 = %d\n", while_num2);
            while_num2++;

            char *current_line = current_line_node->line; // Get the current line
            //printf("Current line_node line: %s\n", current_line_node->line);

            int errCode = parseInput(current_line); // Process the current line
            //printf("Parsed line with error code: %d\n", errCode);
            if (errCode != 0)
            {
                // Print an error message for debugging
                // f//printf(stderr, "Error processing line: %s\n", current_line);
            }

            current_line_node = current_line_node->next; // Advance to next line

            if (current_line_node != NULL)
            {
                //printf("Current line_node after updating to next: %s\n", current_line_node->line);
            }
            else
            {
                //printf("No next line_node found, reached the end of the script.\n");
            }

            //instruction_num++; // Update instruction count
            current_script->current_instruction_num ++;
            instruction_num= current_script->current_instruction_num;
        }

        //printf("Finished processing script for process id: %d\n", current_process->pid);
        //printf("Total instructions executed: %d\n", instruction_num);

        if (current_script->current_instruction_num >= current_script->line_count)
        {
            //printf("Cleaning up process id: %d\n", current_process->pid);
            //printf("Is ready.head == NULL: %d\n", ready.head == NULL); //1 if true
            current_script->current = NULL;
            struct PCB *new_process = get_next_process();
            clean_up_process(current_process);
            current_process = new_process; // Move to the next process
            ////printf("Moving to next process id: %d\n", current_process->pid);

            if (current_process != NULL)
            {
                if(!(new_process->pid == 0 || new_process->pid == 1 || new_process->pid == 2)){
                new_process = NULL;
                break;
            }
                //printf("Moving to next process id: %d\n", current_process->pid);
            }
            else
            {
                //printf("No more processes in the ready queue.\n");
            }
        }
        else
        {
            // If not finished, update the current instruction number in the PCB
            current_script->current_instruction_num = instruction_num;
            current_script->current = current_line_node; // Update to next line node
            //printf("Updated current instruction number: %d for process id: %d\n", instruction_num, current_process->pid);
        }
    }
    script_count = 0;
    ready.head=NULL;
    ready.tail=NULL;
    //printf("Exiting FCFS function\n");
}

void RR()
{
    // Implement Round Robin scheduling here
    //printf("Round Robin scheduling is not implemented yet.\n");
}

int run(char *script)
{
    //printf("inside run\n");

    //printf("Running script: %s\n", script);
    int errCode = 0;
    char line[MAX_USER_INPUT];
    FILE *p = fopen(script, "rt");

    if (p == NULL)
    {
        //fprintf(stderr, "Failed to open script file: %s\n", script);
        return badcommandFileDoesNotExist();
    }

    struct Script *new_script = create_script(script_count); // Initialize the script
    if (new_script == NULL)
    {
        fclose(p);
        //fprintf(stderr, "Failed to create new script\n");
        return errCode;
    }
    //printf("Created new script with count: %d\n", script_count);

    struct PCB *script_pcb = create_pcb(script_count, new_script->current);
    if (script_pcb == NULL)
    {
        fclose(p);
        //fprintf(stderr, "Failed to create PCB for script\n");
        return errCode;
    }
    //printf("script_count after create_pcb: %d\n", script_count);
    script_count--;
    //printf("Created PCB with id: %d\n", script_pcb->pid);
    //add_to_ready_queue(script_pcb);

    while (1)
    {
        if (feof(p))
        {
            //printf("End of script file reached.\n");
            break;
        }
        fgets(line, sizeof(line), p);
        add_line_to_script(new_script, line);
        //printf("Added line to script: %s\n", line);
    }
    fclose(p);

    //FCFS();

    struct Script *current_script = new_script; // Get the process from array
    //printf("Current script head line: %s\n", current_script->head->line);
    struct LineNode *current_line_node = current_script->current;
    int instruction_num = 0; // Local variable for instruction tracking

    int while_num2 = 0;
    //printf("Current script line count: %d\n", current_script->line_count);
    while (instruction_num < current_script->line_count && current_line_node != NULL && while_num2 != 10)
    {
        //printf("Inside FCFS inner while loop. while_num2 = %d\n", while_num2);
        while_num2++;

        char *current_line = current_line_node->line; // Get the current line
        //printf("Current line_node line: %s\n", current_line_node->line);

        int errCode = parseInput(current_line); // Process the current line
        //printf("Parsed line with error code: %d\n", errCode);
        if (errCode != 0)
        {
            // Print an error message for debugging
            // f//printf(stderr, "Error processing line: %s\n", current_line);
        }

        current_line_node = current_line_node->next; // Advance to next line

        if (current_line_node != NULL)
        {
            //printf("Current line_node after updating to next: %s\n", current_line_node->line);
        }
        else
        {
            //printf("No next line_node found, reached the end of the script.\n");
        }

        instruction_num++; // Update instruction count
    }

    //printf("leaving run\n");

    return errCode;
}