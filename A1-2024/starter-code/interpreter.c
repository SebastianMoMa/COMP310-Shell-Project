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

void FCFS()
{
    // printf("inside FCFS\n");
    // int while_num = 0;
    while (ready.head != NULL)
    {
        // printf("inside FCFS while_loop. while_num = %d\n", while_num);
        // while_num++;
        //  Get the current process from the head
        struct PCB *current_process = ready.head;
        // printf("Current process id: %d\n", current_process->pid);
        struct Script *current_script = scripts[current_process->pid]; // Get the process from array
        // printf("Current script head line: %s\n", current_script->head->line);

        // Ensure we start at the first line of the script
        struct LineNode *current_line_node = current_script->head; // Get current line_node
        int instruction_num = 0;                                   // Local variable for instruction tracking

        // Run the process using the current instruction
        // int while_num2 =0;
        // printf("this is instruction_num: %d, this is current_script->line_count: %d\n", instruction_num, current_script->line_count);
        while (instruction_num < current_script->line_count && current_line_node != NULL)
        {
            // printf("inside FCFS while_loop's while loop. while_num2 = %d\n", while_num2);
            // while_num2++;
            char *current_line = current_line_node->line; // Get the current line

            int errCode = parseInput(current_line); // Process the current line
            if (errCode != 0)
            {
                // fprintf(stderr, "Error processing line: %s\n", current_line);
                //  Exit if there's an error in processing
            }

            // Move to the next instruction
            current_line_node = current_line_node->next; // Advance to next line
            instruction_num++;                           // Update instruction count
        }

        // Check if the process is finished
        if (instruction_num >= current_script->line_count)
        {
            // Clean up the process
            free_script(current_script);
            ready.head = current_process->next; // Move to the next process
            int bool_check = ready.head == NULL;
            // printf("Got into instruction_num >= current_script->line_count\nThis is ready.head: %d\n",bool_check);
            free(current_process); // Don't forget to free the current process PCB
        }
        else
        {
            // If not finished, update the current instruction number in the PCB
            current_script->current_instruction_num = instruction_num;
            current_script->current = current_line_node; // Update to next line node
        }
    }
}

int run(char *script)
{
    // printf("Got here baby!!\n");
    int errCode = 0;
    char line[MAX_USER_INPUT];
    FILE *p = fopen(script, "rt"); // the program is in a file

    if (p == NULL)
    {
        return badcommandFileDoesNotExist();
    }
    // Need to find a way to number these for the future ones
    // The pid is their script_count number
    struct Script *new_script = create_script(script_count); // Initialize the script
    if (new_script == NULL)
    {
        fclose(p);
        return errCode; // ????
    }
    scripts[script_count % 3] = new_script; // Putting it into the array for scripts, not sure if this is neccesary yet
    script_count++;

    struct PCB *script_new = (struct PCB *)malloc(sizeof(struct PCB));
    if (script_new == NULL)
    {
        fclose(p);
        free_script(new_script); // Free the allocated script memory
        return errCode;          // This needs to be figured out
        // return memoryAllocationError(); //Don't really have to do this
    }

    script_new->current_instruction = 0;
    script_new->next = NULL;
    script_new->pid = ready.count;

    if (ready.head == NULL)
    {
        // printf("ready.head == NULL\n");
        ready.head = script_new;
        ready.tail = script_new;
    }
    else
    {
        // printf("ready.head != NULL\n");
        ready.tail->next = script_new; // Link the current tail to the new process
        ready.tail = script_new;       // Update tail to the new process
    }
    ready.count++;
    int line_num = 0;
    while (1)
    {
        if (feof(p))
        {
            break;
        }
        fgets(line, sizeof(line), p);
        line_num++;
        add_line_to_script(new_script, line);
        // printf("This is the line added: %s\n", new_script->head->next->next->line);
    }
    fclose(p);

    FCFS();
    return errCode;
}
