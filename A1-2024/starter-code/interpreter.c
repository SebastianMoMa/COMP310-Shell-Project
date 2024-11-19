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

int MAX_ARGS_SIZE = 7;
int Background_happening = 0;
char *background_policy = "";

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
int exec(char *processes[], int numProcesses, char *policy, int isBackground);
void Scheduler1();
void load_queue_FCFS();
void load_queue_SJF();
void SJF();
void swap_scripts(struct Script **a, struct Script **b);
void RR(int numCommands);
void Aging();
void printFramesForScript(int processId);
void ageUsedFrames();
void shift_queue();
int findLRU();
void printAgesFrames();

// Interpret commands and their arguments
int interpreter(char *command_args[], int args_size)
{
    // printf("here interpreter\n");
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
        // return help();
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
        // printf("here set\n");
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
        // printf("going into set, var: %s, value: %s\n", command_args[1], value);
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
        if (args_size > 6)
        {
            return badcommandtooManyProcesses();
        }

        int isBackground = 0;
        if (strcmp(command_args[args_size - 1], "#") == 0)
        {
            isBackground = 1;
            Background_happening = 1;
            background_policy = command_args[args_size - 1 - isBackground];
        }
        char *processes[args_size - 2 - isBackground];
        int numProcesses = 0;
        // printf("arg_size: %d\n", args_size);

        for (int i = 1; i < args_size - 1 - isBackground; i++)
        {
            // printf("process: %s\n", command_args[i]);
            processes[i - 1] = command_args[i];
            numProcesses++;
        }
        return exec(processes, numProcesses, command_args[args_size - 1 - isBackground], isBackground);
        // return exec(processes, numProcesses, command_args[args_size - 1]);
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
    // printf("Did set!\n");
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

int checkEOF(struct Script *script)
{
    char *process = script->name;
    int errCode = 0;
    FILE *backingStore = fopen(process, "rt");
    // printf("process: %s\n", process);
    if (backingStore == NULL)
    {
        return badcommandFileDoesNotExist(); // Handle file not found error
    }

    int same_name = lookForName(script_count, process);
    script = NULL;
    int id = 0;
    // if (same_name == -1)
    // {
    //     printf("New Script\n");
    //     script = create_script(script_count, process); // Initialize the script if new
    //     id = script_count;
    // }
    // else
    // {
    script = scripts[same_name];
    id = same_name;
    //}

    int positionFile = script->offset;
    // printf("Position in file: %d\n", positionFile);

    // Use fgetc to check if it's the end of the file
    fseek(backingStore, positionFile, SEEK_SET); // Position to the start of the page

    long currentPos = ftell(backingStore);

    if (fgetc(backingStore) == EOF && feof(backingStore))
    {
        printf("End of file reached at position: %ld\n", currentPos);
        fclose(backingStore);
        return 1; // Handle as needed
    }

    // Reset the file pointer to the original position
    // fseek(backingStore, currentPos, SEEK_SET);
    fseek(backingStore, positionFile, SEEK_SET);
    fclose(backingStore);
    return -1;
}

int loadPageToFrameStore(char *process)
{
    int errCode = 0;
    char line[100];
    FILE *backingStore = fopen(process, "rt");
    // printf("process: %s\n", process);
    if (backingStore == NULL)
    {
        return badcommandFileDoesNotExist(); // Handle file not found error
    }

    int same_name = lookForName(script_count, process);
    struct Script *script = NULL;
    int id = 0;
    if (same_name == -1)
    {
        // printf("New Script\n");
        script = create_script(script_count, process); // Initialize the script if new
        id = script_count;
    }
    else
    {
        script = scripts[same_name];
        id = same_name;
    }

    if (script == NULL)
    {
        fclose(backingStore);
        return errCode; // Error if the script could not be created
    }

    int freeFrame = findFreeFrame();
    // printf("Free frame: %d\n", freeFrame);

    if (freeFrame == -1)
    {
        // printAgesFrames();
        freeFrame = findLRU();
        // printf("frame to replace number: %d\n", freeFrame);
        int frameID = frameStore[freeFrame].processId;
        for (int k = 0; k < 3; k++)
            {
                //printf("Script: %d, frame: %d, Line%d: %s\n", id, freeFrame, k, frameStore[freeFrame].lines[k]);
                printf("%s\n",frameStore[freeFrame].lines[k]);

            }
        // for (int i = 0; i < scripts[frameID]->totalPages; i++)
        // {

        //     int frameNumber = pageTable[frameID][i];

        //     for (int k = 0; k < 3; k++)
        //     {
        //         printf("Script: %d, frame: %d, Line%d: %s\n", id, i, k, frameStore[frameNumber].lines[k]);
        //     }
        // }
        printf("End of victim page contents.\n");
        // return 0;
        //  We will implement this later with LRU
        //  freeFrame = selectFrameToReplace(); // Select a frame to replace if no free frames are available
    }

    // Use ftell() to get the current position within the file. Each time you load a page,
    // record the position (in bytes) where you stop reading.
    // Later, you can use fseek() to move back to this exact spot in the file.

    int positionFile = script->offset;
    // printf("Position in file: %d\n", positionFile);

    // Use fgetc to check if it's the end of the file
    fseek(backingStore, positionFile, SEEK_SET); // Position to the start of the page

    long currentPos = ftell(backingStore);

    if (fgetc(backingStore) == EOF && feof(backingStore))
    {
        // printf("End of file reached at position: %ld\n", currentPos);
        fclose(backingStore);
        return errCode; // Handle as needed
    }
    frameStore[freeFrame].linesUsed = 0;
    // Reset the file pointer to the original position
    // fseek(backingStore, currentPos, SEEK_SET);

    fseek(backingStore, positionFile, SEEK_SET); // Position to the start of the page
    for (int i = 0; i < 3; i++)
    {
        if (fgets(line, sizeof(line), backingStore) == NULL)
        {
            // printf("NUll line: %s\n", line);
            strcpy(frameStore[freeFrame].lines[i], "-1"); // Empty line if end-of-file
            break;
        }
        else
        {
            //printf("line: %s\n", line);
            strcpy(frameStore[freeFrame].lines[i], line);
            frameStore[freeFrame].linesUsed++;
            script->offset += strlen(line);
            // printf("Updated offset: %d\n", script->offset);
        }
    }

    frameStore[freeFrame].pageNumber = script->totalPages;

    frameStore[freeFrame].processId = id;
    // This needs to be implemented later to keep track of least used
    frameStore[freeFrame].lastUsed = 0;

    // Update page table and script’s frame table
    // printf("Script count1: %d\n", script_count);
    pageTable[id][script->totalPages] = freeFrame;
    // printf("Script: %d, newframenumber: %d frame: %d\n", id, script->totalPages, pageTable[id][script->totalPages]);
    script->totalPages++;
    if (same_name == -1)
    {
        struct PCB *script_pcb = create_pcb(id, script->current); // Create a PCB for new script
        if (script_pcb == NULL)
        {
            fclose(backingStore);
            return errCode;
        }
    }

    // printf("Script count2: %d\nscript page number: %d\n", script_count, script->totalPages);

    // for (int j = 0; j < script_count; j++)
    // {
    //     struct Script *scripity = scripts[j];

    //     for (int i = 0; i < scripity->totalPages; i++)
    //     {
    //         int frameNumber = pageTable[j][i];
    //         printf("frame number: %d\n", frameNumber);
    //         for (int k = 0; k < 3; k++){
    //             printf("Script: %d, frame: %d, Line%d: %s\n",j,i,k, frameStore[frameNumber].lines[k]);
    //         }
    //     }

    //     //printFramesForScript(0);

    // }
    fclose(backingStore);
    return 0; // Success
}

void printFramesForScript(int processId)
{

    printf("\nFrames for Process ID: %d\n", processId);
    printf("| Frame # | Page # | Last Used  | Lines in Frame       |\n");

    int found = 0;

    for (int i = 0; i < FRAME_STORE_SIZE; i++)
    {
        if (frameStore[i].processId == processId)
        {
            found = 1;
            printf("| %-7d | %-6d | %-9d | %-20s |\n",
                   i,
                   frameStore[i].pageNumber,
                   frameStore[i].lastUsed,
                   frameStore[i].lines[0]);                                               // First line in the frame
            printf("|         |        |           | %-20s |\n", frameStore[i].lines[1]); // Second line
            printf("|         |        |           | %-20s |\n", frameStore[i].lines[2]); // Third line
        }
    }

    if (!found)
    {
        printf("No frames found for Process ID %d.\n", processId);
    }
}

void printallFramesForScript()
{

    int found = 0;
    for (int j = 0; j < script_count; j++)
    {
        printf("\nFrames for Process ID: %d\n", j);
        printf("| Frame # | Page # | Last Used  | Lines in Frame       |\n");

        for (int i = 0; i < FRAME_STORE_SIZE; i++)
        {
            if (frameStore[i].processId == j)
            {
                found = 1;
                printf("| %-7d | %-6d | %-9d | %-20s |\n",
                       i,
                       frameStore[i].pageNumber,
                       frameStore[i].lastUsed,
                       frameStore[i].lines[0]);                                               // First line in the frame
                printf("|         |        |           | %-20s |\n", frameStore[i].lines[1]); // Second line
                printf("|         |        |           | %-20s |\n", frameStore[i].lines[2]); // Third line
            }
            if (!found)
            {
                printf("No frames found for Process ID %d.\n", j);
            }
        }
    }
}

/// home/2023/smoser4/FALL2024/COMP310/A1/comp310-ecse427-coursework-f24/A1-2024/starter-code

// script_num should be field in Frame
int sendLinesToScript(int script_num, int pageNum)
{
    struct Script *script = scripts[script_num];

    // int pageNum = page;                                   // Page number based on current instruction
    // int lineOffset = script->current_instruction_num % 3; // Line within the page

    int frameIdx = pageTable[script_num][pageNum];
    // printf("script: %d page: %d Frame index: %d\n", script_num, pageNum, frameIdx);

    if (frameIdx == -1)
    {
        printf("Error: Page not loaded in frame store.\n");
        return -1;
    }

    // printf("frameStore[frameIdx].linesUsed: %d\n", frameStore[frameIdx].linesUsed);
    for (int i = 0; i < frameStore[frameIdx].linesUsed; i++)
    {
        // printf("Line to add: %s\n", frameStore[frameIdx].lines[i]);
        add_line_to_script(script, frameStore[frameIdx].lines[i]);
    }

    // Update script's current instruction number
    // script->current_instruction_num += linesToSend;

    return 0;
}

int loadProcessestoMemory(char *process)
{
    // printf("loading script: %s\n", process);
    int errCode = 0;
    char line[MAX_USER_INPUT];
    FILE *p = fopen(process, "rt");

    if (p == NULL)
    {
        // f//printf(stderr, "Failed to open script file: %s\n", process);
        return badcommandFileDoesNotExist();
    }

    int same_name = lookForName(script_count, process);
    struct Script *new_script = NULL;
    if (same_name == -1)
    {

        new_script = create_script(script_count, process); // Initialize the script
    }
    else
    {
        new_script = scripts[same_name];
    }
    if (new_script == NULL)
    {
        fclose(p);
        // f//printf(stderr, "Failed to create new script\n");
        return errCode;
    }
    // printf("Created new script with count: %d\n", script_count);

    while (1) //(fgets(line, sizeof(line), p) !=NULL)
    {
        if (fgets(line, sizeof(line), p) == NULL)
        {
            add_line_to_script(new_script, " ");
            printf("This is line where fgets(line, sizeof(line), p) == NULL: %s\n", line);
            break;
        }
        if (feof(p))
        {
            // printf("This is last line: %s\n", line);
            add_line_to_script(new_script, line);
            break;
        }
        // printf("This is line: %s\n", line);
        add_line_to_script(new_script, line);
        // printf("Added line to script: %s\n", line);
    }
    struct PCB *script_pcb = NULL;
    if (same_name == -1)
    {
        struct PCB *script_pcb = create_pcb(script_count, new_script->current);
        if (script_pcb == NULL)
        {
            fclose(p);
            // f//printf(stderr, "Failed to create PCB for script\n");
            return errCode;
        }
    }

    // printf("script_count: %d\n", script_count);

    // printf("Created PCB with id: %d\n", script_pcb->pid);
    // add_to_ready_queue(script_pcb);
    // struct PCB *for_pcb = ready.head;
    // for (int i =0; i < script_count; i++){
    // printf("This is pcb %d in ready queue: %s\n", i, for_pcb->current->line);
    //    for_pcb = for_pcb->next;
    //}
    // printf("leaving loadProcessestoMemory\n");

    fclose(p);
}

int exec(char *processes[], int numProcesses, char *policy, int isBackground)
{
    if (Background_happening == 1)
    {
        char *policy = background_policy;
    }
    int errcode = 0;
    // printf("This is numProcesses: %d\n", numProcesses);
    // printf("This is the policy: '%s'. script_num: %d\n", policy, numProcesses);

    // if (numProcesses == 2)
    // {
    //     if (strcmp(processes[0], processes[1]) == 0)
    //     {
    //         return badcommand();
    //     }
    // }
    // if (numProcesses == 3)
    // {
    //     // if either of the processes are the same, we should give an error
    //     if (strcmp(processes[0], processes[1]) == 0 || strcmp(processes[0], processes[2]) == 0 || strcmp(processes[2], processes[1]) == 0)
    //     {
    //         return badcommand();
    //     }
    // }

    if (!(strcmp(policy, "FCFS") == 0 || strcmp(policy, "SJF") == 0 || strcmp(policy, "RR") == 0 || strcmp(policy, "AGING") == 0 || strcmp(policy, "RR30") == 0))
    {
        // printf("Error: Invalid Policy\n");
        return 4;
    }

    initialize_frame_store();

    if (numProcesses == 1 && isBackground != 1 && Background_happening != 1)
    {
        // printf("Loading single process: %s\n", processes[0]);

        // I guess the idea here is to give it the # of pages that the process is going to use
        // int frameNumber = findFreeFrame();

        loadPageToFrameStore(processes[0]);
        sendLinesToScript(0, 0);
        // ageUsedFrames();
        loadPageToFrameStore(processes[0]);
        sendLinesToScript(0, 1);
        // ageUsedFrames();
        // printAgesFrames();
        // loadProcessestoMemory(processes[0]);
        load_queue_FCFS(numProcesses);
        // printf("Here\n");

        // printf("Executing Scheduler1 for single process.\n");
        Scheduler1();
        return 0;
    }
    else if (numProcesses == 1 && isBackground != 1 && Background_happening == 1)
    {
        // printf("2Loading single process: %s\n", processes[0]);
        // int frameNumber = findFreeFrame();
        loadPageToFrameStore(processes[0]);
        sendLinesToScript(0, 0);
        ageUsedFrames();
        loadPageToFrameStore(processes[0]);
        sendLinesToScript(0, 1);
        ageUsedFrames();
        // loadProcessestoMemory(processes[0]);
        load_queue_FCFS(numProcesses);
    }

    else
    {
        // printf("Loading multiple processes...\n");
        for (int i = 0; i < numProcesses; i++)
        {
            // printf("Got here\n");
            // int frameNumber = findFreeFrame();
            loadPageToFrameStore(processes[i]);
            sendLinesToScript(i, 0);
            // ageUsedFrames();
            loadPageToFrameStore(processes[i]);
            sendLinesToScript(i, 1);
            // ageUsedFrames(); // I think this might work, but I need to think about it a bit more
            //  printFramesForScript(i);

            // loadProcessestoMemory(processes[i]);
        }

        printallFramesForScript();

        if (strcmp(policy, "FCFS") == 0)
        {

            load_queue_FCFS(numProcesses);
            Scheduler1();
        }
        else if (strcmp(policy, "SJF") == 0)
        {
            load_queue_SJF(numProcesses);
            Scheduler1();
        }
        else if (strcmp(policy, "RR") == 0)
        {
            load_queue_FCFS(script_count);

            printf("Going into RR\n");
            RR(2);
            printf("Leaving into RR\n");
        }
        else if (strcmp(policy, "AGING") == 0)
        {
            // printf("Going into Aging\n");
            // InfoAboutPCBsandScripts(script_count);
            load_queue_SJF(numProcesses);
            // InfoAboutPCBsandScripts(script_count);
            Aging();
        }
        else if (strcmp(policy, "RR30") == 0)
        {
            load_queue_FCFS(numProcesses);
            RR(30);
            return errcode;
        }
    }
    printf("Leaving totally\n");
}

void load_queue_FCFS(int num_processes)
{
    struct PCB *script_pcb = NULL;
    printf("this is script_count: %d\n", script_count);

    for (int i = 0; i < script_count; i++)
    {
        script_pcb = PCBs[i];
        script_pcb->job_length_score = scripts[script_pcb->pid]->job_length_score;
        // printf("this is pid: %d job_length_score %d\n",script_pcb->pid, script_pcb->job_length_score); //Load Job_length scores here
        //  printf("this is PCB %s\n", scripts[script_pcb->pid]->head->line);
        add_to_ready_queue(script_pcb);
        printf("Added PCB with id: %d, and scripts number: %d to the ready queue.\n", script_pcb->pid, i);
    }
    // printf("leaving load_queue_FCFS\n");
}

void ageUsedFrames()
{
    for (int i = 0; i < FRAME_STORE_SIZE / 3; i++)
    {
        if (frameStore[i].Started == 1)
        {
            // printf("Aging frame: %d", i);
            frameStore[i].lastUsed++;
            // printf("to age: %d\n", frameStore[i].lastUsed);
        }
    }
}

void printAgesFrames()
{
    for (int i = 0; i < FRAME_STORE_SIZE / 3; i++)
    {
        printf("Frame: %d, age: %d\n", i, frameStore[i].lastUsed);
    }
}

int findLRU()
{
    int max = 0;
    int maxAge = 0;
    for (int i = 0; i < FRAME_STORE_SIZE / 3; i++)
    {
        if (frameStore[i].lastUsed > maxAge)
        {
            max = i;
            maxAge = frameStore[i].lastUsed;
        }
    }
    return max;
}

// round robin, versatile to number of commands passed
void RR(int numCommands)
{

    int while_num = 0;
    // struct PCB *current_process = ready.head;
    while (ready.head != NULL)
    {
        ageUsedFrames();
        // printAgesFrames();
        struct PCB *current_process = ready.head;
        // printf("Inside RR while_loop. while_num = %d\n", while_num);
        while_num++;

        // If current process is invalid, break
        if (current_process == NULL)
            break;

        struct Script *current_script = scripts[current_process->pid]; // Get the process from array
        //printf("Current script name: %s\n", current_script->name);
        struct LineNode *current_line_node = current_script->current;
        int instruction_num = current_script->current_instruction_num; // Local variable for instruction tracking
        // printf("Current script line_num: %d\n", instruction_num);

        int while_num2 = 0;
        //  printf("Current script line count: %d, current instruction_num: %d\n", current_script->line_count, instruction_num);
        // printf("Current line node == NULL: %d\n", current_line_node ==NULL);

        while (instruction_num < current_script->line_count && current_line_node != NULL && while_num2 != numCommands)
        {
            //  printf("Inside RR inner while loop. while_num2 = %d\n", while_num2);
            while_num2++;

            char *current_line = current_line_node->line; // Get the current line
            // printf("Current line_node line: %s", current_line_node->line);

            int errCode = parseInput(current_line); // Process the current line
                                                    // printf("Parsed line with error code: %d\n", errCode);

            current_line_node = current_line_node->next; // Advance to next line

            // instruction_num++; // Update instruction count
            current_script->current = current_line_node;
            current_script->current_instruction_num++;
            instruction_num = current_script->current_instruction_num;

            int end = checkEOF(current_script);
            if (!(current_script->current_instruction_num >= current_script->line_count))
            {
                int pageOfScript = instruction_num / 3;
                // if( instruction_num >3){
                //     pageOfScript = pageOfScript + instruction_num%3;
                // }
                int scriptNum = lookForName(script_count, current_script->name);
                // int scriptNum1 = current_process->pid;
                int frameIndex = pageTable[scriptNum][pageOfScript];
                // printf("Line number in script: %d\n", instruction_num);
                // printf("Right before age = 0 again, Script: %d, page: %d, frame: %d\n", scriptNum, pageOfScript, frameIndex);
                frameStore[frameIndex].Started = 1;
                frameStore[frameIndex].lastUsed = 0;
            }
        }
        //  printf("Finished processing script for process id: %d\n", current_process->pid);
        // printf("Total instructions executed: %d\n", instruction_num);
        if (current_script->current_instruction_num >= current_script->line_count)
        {
            // printf("Total instructions executed: %d, number of lines in script: %d\n", instruction_num, current_script->line_count);

            if (current_script->readyToChange == 0)
            {
                shift_queue();
                current_script->readyToChange = 1;
                // printf("Gonna continue\n");
                continue;
            }
            current_script->readyToChange = 0;

            int end = checkEOF(current_script);
            if (end != 1)
            { // meaning out of frames from this script in framestore, but more to put in from backing store

                int freeFrame = findFreeFrame();
                if (freeFrame == -1)
                {
                    //printf("Frame store full because free frame: %d\n", freeFrame);
                    printf("Page fault! Victim page contents:\n");
                }
                else
                {
                    

                    printf("Page fault!\n");
                }
                int scriptNum = lookForName(script_count, current_script->name);

                loadPageToFrameStore(current_script->name);
                int totalPagesforScript = current_script->totalPages;
                // printf("totalPagesforScript: %d!\n", totalPagesforScript);
                sendLinesToScript(scriptNum, totalPagesforScript-1);
                instruction_num = current_script->line_count;
                
                //continue;
                shift_queue;
            }
            else if (end == 1)
            {
                // This is exactly where we want to check if there are more frames for this script
                //, and if so then put them in via LRU

                printf("Cleaning up process id: %d\n", current_process->pid);
                // printf("Is ready.head == NULL: %d\n", ready.head == NULL); //1 if true
                current_script->current = NULL;
                struct PCB *new_process = get_next_process();
                clean_up_process(current_process);
                current_process = new_process; // Move to the next process
                ////printf("Moving to next process id: %d\n", current_process->pid);
                if (current_process != NULL)
                {
                    if (!(new_process->pid == 0 || new_process->pid == 1 || new_process->pid == 2))
                    {
                        new_process = NULL;
                        break;
                    }
                    // printf("Moving to next process id: %d\n", current_process->pid);
                }
            }
        }
        else
        {
            shift_queue();
        }
    }
    script_count = 0;
    ready.head = NULL;
    ready.tail = NULL;
    // printf("Exiting RR function\n");
}

void shift_queue()
{
    if (ready.count < 2)
    {
        // No need to shift if there are fewer than 2 elements
        return;
    }

    struct PCB *old_head = ready.head;
    struct PCB *old_tail = ready.tail;

    // Rearranging the pointers
    struct PCB *new_head = old_head->next;
    struct PCB *new_tail = old_head;

    if (ready.count == 2)
    {
        new_head->next = new_tail;
        new_tail->next = NULL;
    }
    if (ready.count == 3)
    {
        new_head->next->next = new_tail;
        new_tail->next = NULL;
    }

    // Update the queue
    ready.head = new_head; // New head is now middle
    ready.tail = new_tail; // Old head becomes the new tail
}

void Scheduler1() // Works for FCFS and SJF
{
    // printf("Starting Scheduler1 function\n");
    int while_num = 0;
    struct PCB *current_process = ready.head;
    while (ready.head != NULL)
    {
        // printf("Inside Scheduler1 while_loop. while_num = %d\n", while_num);
        while_num++;
        // Check if current_process is valid, otherwise break
        if (current_process == NULL)
            break;
        struct Script *current_script = scripts[current_process->pid]; // Get the process from array
        // printf("Current script head line: %s\n", current_script->head->line);
        struct LineNode *current_line_node = current_script->current;
        int instruction_num = current_script->current_instruction_num; // Local variable for instruction tracking
        // printf("Current script line_num: %d\n", instruction_num);

        int while_num2 = 0;
        // printf("Current script line count: %d\n", current_script->line_count);
        while (instruction_num < current_script->line_count && current_line_node != NULL)
        {
            // printf("Inside Scheduler1 inner while loop. while_num2 = %d\n", while_num2);
            while_num2++;
            char *current_line = current_line_node->line; // Get the current line
            // printf("Current line_node line: %s\n", current_line_node->line);
            int errCode = parseInput(current_line); // Process the current line
            // printf("Parsed line with error code: %d\n", errCode);
            current_line_node = current_line_node->next; // Advance to next line
            // instruction_num++; // Update instruction count
            current_script->current_instruction_num++;
            instruction_num = current_script->current_instruction_num;
        }
        // printf("Finished processing script for process id: %d\n", current_process->pid);
        // printf("Total instructions executed: %d\n", instruction_num);
        if (current_script->current_instruction_num >= current_script->line_count)
        {
            // printf("Cleaning up process id: %d\n", current_process->pid);
            // printf("Is ready.head == NULL: %d\n", ready.head == NULL); //1 if true
            current_script->current = NULL;
            struct PCB *new_process = get_next_process();
            clean_up_process(current_process);
            script_count--;
            current_process = new_process; // Move to the next process
            ////printf("Moving to next process id: %d\n", current_process->pid);

            if (current_process != NULL)
            {
                if (!(new_process->pid == 0 || new_process->pid == 1 || new_process->pid == 2))
                {
                    new_process = NULL;
                    break;
                }
                // printf("Moving to next process id: %d\n", current_process->pid);
            }
        }
    }
    script_count = 0;
    ready.head = NULL;
    ready.tail = NULL;
    // printf("Exiting Scheduler1 function\n");
}

int compare_ints(int a, int b)
{
    return a - b; // Return negative, 0, or positive
    // negative means a is smaller, so b is bigger
    // positive means b is smaller, and yeah a is bigger then
    // 0 means ==
}

int AgeJobs()
{
    struct PCB *head = ready.head;
    struct PCB *tail = ready.tail;
    // printf("Got here in agejobs. Script_count: %d\n", script_count);
    if (script_count == 2)
    {
        // decrement the job length scores, perform the necessary operations to switch the head/tail based on comparison between head and head->next job length score
        if (head->next->job_length_score > 0)
        {
            head->next->job_length_score--;
            scripts[head->next->pid]->job_length_score--;
        }
        if (head->job_length_score > head->next->job_length_score)
        {
            ready.head = head->next;
            ready.tail = head;
            ready.head->next = ready.tail;
            ready.tail->next = NULL;
        }
    }
    else if (script_count == 3)
    {
        int to_check = compare_ints(head->next->job_length_score, tail->job_length_score);

        if (head->next->job_length_score > 0)
        {
            head->next->job_length_score--; // Decrease middle job_length_score
            scripts[head->next->pid]->job_length_score--;
        }
        if (tail->job_length_score > 0)
        {
            tail->job_length_score--;
            scripts[tail->pid]->job_length_score--;
        }
        // Below tells us which of the two non-head PCBs's have lower job_length_score

        if ((to_check < 0))
        { // so middle is smaller
            if (head->job_length_score > head->next->job_length_score)
            {
                shift_queue();
            }
        }

        if (to_check == 0)
        {
            if (ready.head->next->pid < ready.tail->pid)
            {
                if (head->job_length_score > ready.head->next->job_length_score)
                {

                    shift_queue();
                }
            }
            else
            {
                if (head->job_length_score > tail->job_length_score)
                {
                    ready.head = tail;
                    ready.head->next = head->next;
                    ready.tail = head;
                    ready.head->next->next = ready.tail;
                    ready.tail->next = NULL;
                }
            }
        }
        else
        {
            if (head->job_length_score > tail->job_length_score)
            {
                ready.head = tail;
                ready.head->next = head->next;
                ready.tail = head;
                ready.head->next->next = ready.tail;
                ready.tail->next = NULL;
            }
        }
    }
}

void Aging()
{
    int while_num = 0;
    int original_script_count = script_count;
    // int enqueued = 0;
    //  struct PCB *current_process = ready.head;
    while (ready.head != NULL)
    {
        struct PCB *current_process = ready.head;
        // printf("Inside Aging while_loop. while_num = %d\n", while_num);
        while_num++;
        // Check if current_process is valid, otherwise break
        if (current_process == NULL)
            break;

        struct Script *current_script = scripts[current_process->pid]; // Get the process from array
        // printf("Current script line: %s\n", current_script->current->line);
        struct LineNode *current_line_node = current_script->current;
        int instruction_num = current_script->current_instruction_num; // Local variable for instruction tracking
        // printf("Current script line_num: %d\n", instruction_num);

        int while_num2 = 0;
        // printf("Current script line count: %d\n", current_script->line_count);
        while (instruction_num < current_script->line_count && current_line_node != NULL && while_num2 != 1)
        {
            // printf("Inside Aging inner while loop. while_num2 = %d\n", while_num2);
            while_num2++;

            char *current_line = current_line_node->line; // Get the current line
            // printf("Current line_node line: %s\n", current_line_node->line);

            int errCode = parseInput(current_line); // Process the current line
            // printf("Parsed line with error code: %d\n", errCode);
            if (errCode != 0)
            {
                // Print an error message for debugging
                // f//printf(stderr, "Error processing line: %s\n", current_line);
            }
            current_line_node = current_line_node->next; // Advance to next line
            if (current_line_node != NULL)
            {
                // printf("Current line_node after updating to next: %s\n", current_line_node->line);
            }
            else
            {
                // printf("No next line_node found, reached the end of the script.\n");
            }
            // instruction_num++; // Update instruction count
            current_script->current = current_line_node;
            current_script->current_instruction_num++;
            instruction_num = current_script->current_instruction_num;
        }
        // printf("Finished processing script for process id: %d\n", current_process->pid);
        // printf("Total instructions executed: %d\n", instruction_num);

        if (current_script->current_instruction_num >= current_script->line_count)
        {
            // printf("Cleaning up process id: %d\n", current_process->pid);
            // printf("Is ready.head == NULL: %d\n", ready.head == NULL); //1 if true
            current_script->current = NULL;
            // printf("Before Aging:\n");
            // printJobLengthScore(original_script_count);
            Once_Done_AGING();
            // AgeJobs();
            // struct PCB *new_process = get_next_process();
            // printf("Got here\n");
            // clean_up_process(current_process);
            // script_count--;
            // current_process = new_process; // Move to the next process
            current_process = ready.head;
            if (ready.head == NULL)
                break;
            // printf("Got here\n");
            // AgeJobs();

            //  printf("Just finished one script. New Process: %s", current_process->current->line);
            // printf("After Aging:\n");
            // printJobLengthScore(original_script_count);

            ////printf("Moving to next process id: %d\n", current_process->pid);

            if (current_process != NULL)
            {
                if (!(current_process->pid == 0 || current_process->pid == 1 || current_process->pid == 2))
                {
                    current_process = NULL;
                    break;
                }
                // printf("Moving to next process id: %d\n", current_process->pid);
            }
        }
        else
        {
            AgeJobs();
        }
    }
    script_count = 0;
    ready.head = NULL;
    ready.tail = NULL;
    //  printf("Finished Aging\n");
}

void load_queue_SJF(int numProcesses)
{
    struct PCB *script_pcb = NULL;
    // printf("this is script_count: %d\n", script_count);

    sortScriptsByLineCount(numProcesses);
    // InfoAboutPCBsandScripts(script_count);
    // printScripts(script_count);

    for (int i = 0; i < numProcesses; i++)
    {
        script_pcb = PCBs[i];
        script_pcb->job_length_score = scripts[script_pcb->pid]->job_length_score;
        // printf("this is pid: %d's job_length_score %d\n", script_pcb->pid, script_pcb->job_length_score); //Load Job_length scores here
        // printf("this is pid: %d's head line: %s\n", script_pcb->pid, scripts[script_pcb->pid]->head->line);
        add_to_ready_queue(script_pcb);
        // printf("Added PCB with id: %d to the ready queue.\n", script_pcb->pid);
    }
    // printf("leaving load_queue_FCFS\n");
}

int run(char *script)
{
    // printf("Running script: %s\n", script);
    int errCode = 0;
    char line[MAX_USER_INPUT];
    FILE *p = fopen(script, "rt");
    if (p == NULL)
    {
        // fprintf(stderr, "Failed to open script file: %s\n", script);
        return badcommandFileDoesNotExist();
    }
    struct Script *new_script = create_script(script_count, script); // Initialize the script
    if (new_script == NULL)
    {
        fclose(p);
        // fprintf(stderr, "Failed to create new script\n");
        return errCode;
    }
    // printf("Created new script with count: %d\n", script_count);

    struct PCB *script_pcb = create_pcb(script_count, new_script->current);
    if (script_pcb == NULL)
    {
        fclose(p);
        // fprintf(stderr, "Failed to create PCB for script\n");
        return errCode;
    }
    // printf("script_count after create_pcb: %d\n", script_count);
    script_count--;
    // printf("Created PCB with id: %d\n", script_pcb->pid);
    // add_to_ready_queue(script_pcb);

    while (1) //(fgets(line, sizeof(line), p) !=NULL)
    {
        if (fgets(line, sizeof(line), p) == NULL)
        {
            add_line_to_script(new_script, " ");
            // printf("This is line where fgets(line, sizeof(line), p) ==NULL: %s\n", line);
            break;
        }
        if (feof(p))
        {
            // printf("This is last line: %s\n", line);
            add_line_to_script(new_script, line);
            break;
        }
        // printf("This is line: %s\n", line);
        add_line_to_script(new_script, line);
        // printf("Added line to script: %s\n", line);
    }
    fclose(p);

    struct Script *current_script = new_script; // Get the process from array
    // printf("Current script head line: %s\n", current_script->head->line);
    struct LineNode *current_line_node = current_script->current;
    int instruction_num = 0; // Local variable for instruction tracking

    int while_num2 = 0;
    // printf("Current script line count: %d\n", current_script->line_count);
    while (instruction_num < current_script->line_count && current_line_node != NULL && while_num2 != 10)
    {
        // printf("Inside Scheduler1 inner while loop. while_num2 = %d\n", while_num2);
        while_num2++;
        char *current_line = current_line_node->line; // Get the current line
        // printf("Current line_node line: %s\n", current_line_node->line);
        int errCode = parseInput(current_line); // Process the current line
        // printf("Parsed line with error code: %d\n", errCode);
        current_line_node = current_line_node->next; // Advance to next line
        instruction_num++;                           // Update instruction count
    }
    return errCode;
}