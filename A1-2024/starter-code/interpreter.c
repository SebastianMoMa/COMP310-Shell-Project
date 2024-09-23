#include <stdio.h>
#include <stdlib.h>
#include <string.h> 
#include "shellmemory.h"
#include "shell.h"

int MAX_ARGS_SIZE = 8;

int badcommand(){
    printf("Unknown Command\n");
    return 1;
}

int badcommandTooManyTokens(){
    printf("Bad command: Too many tokens\n");
    return 2;
}

// For run command only
int badcommandFileDoesNotExist(){
    printf("Bad command: File not found\n");
    return 3;
}

int help();
int quit();
int set(char* var, char* value);
int print(char* var);
int run(char* script);
int echo(char* value);
int badcommandFileDoesNotExist();

// Interpret commands and their arguments
int interpreter(char* command_args[], int args_size) {
    int i;
    //printf("args_size 1: %d\n", args_size);

    if (args_size < 1 || args_size > MAX_ARGS_SIZE) {
        return badcommand();
    }

    for (i = 0; i < args_size; i++) { // terminate args at newlines
        command_args[i][strcspn(command_args[i], "\r\n")] = 0;
    }

    if (strcmp(command_args[0], "help") == 0){
        //help
        if (args_size != 1) return badcommand();
        return help();
    
    } else if (strcmp(command_args[0], "quit") == 0) {
        //quit
        if (args_size != 1) return badcommand();
        return quit();

    } else if (strcmp(command_args[0], "set") == 0) {
        //set
        //printf("args_size 2: %d\n", args_size);
        if (3 > args_size) {
            //printf("3>argsize");
            return badcommand();}
        else if	(args_size>7) return badcommandTooManyTokens();
       // printf("I am here and command_args[2] is %s, and command_args[3] is %s \n", command_args[2], command_args[3]);
    char value[MAX_USER_INPUT] = "";
    for (int i = 2; i < args_size; i++) {
        if (i > 2) strcat(value, " "); // Add space if not the first token
        strcat(value, command_args[i]);
    }
    return set(command_args[1], value);

    // print
    } else if (strcmp(command_args[0], "print") == 0) {
        if (args_size != 2) return badcommand();
        return print(command_args[1]);
    
    } 
    // run
    else if (strcmp(command_args[0], "run") == 0) {
        if (args_size != 2) return badcommand();
        return run(command_args[1]);
    
    } 

    else if (strcmp(command_args[0], "echo") ==0) {
        if (args_size != 2) return badcommand();
        return echo(command_args[1]);
    }
    
    else return badcommand();
}

int help() {

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

int quit() {
    printf("Bye!\n");
    exit(0);
}

int set(char *var, char *value) {
   // printf("var: %s, value: %s\n", var, value);

    mem_set_value(var, value);

    return 0;
}

int echo(char *arg) {

    if (arg[0] == '$'){
        char* var_name = arg + 1;
        char* value = mem_get_value(var_name);
        //printf("this is value: %s\n", value);
        if (strcmp("Variable does not exist", value) !=0) {
           // printf("strcmp returned !0 so they are value != Variable does not exist ");
            printf("%s\n", value); // Print the value if it exists
        } else {
            printf("\n"); // Print a blank line if the variable is not found
        }
    }
    else {
        printf("%s\n", arg); // Print the argument directly
    }

    return 0;
}


int print(char *var) {
    printf("%s\n", mem_get_value(var)); 
    return 0;
}

int run(char *script) {
    int errCode = 0;
    char line[MAX_USER_INPUT];
    FILE *p = fopen(script, "rt");  // the program is in a file

    if (p == NULL) {
        return badcommandFileDoesNotExist();
    }

    fgets(line, MAX_USER_INPUT-1, p);
    while (1) {
        errCode = parseInput(line);	// which calls interpreter()
        memset(line, 0, sizeof(line));

        if (feof(p)) {
            break;
        }
        fgets(line, MAX_USER_INPUT-1, p);
    }

    fclose(p);

    return errCode;
}
