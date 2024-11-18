//shell.c
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "shell.h"
#include "interpreter.h"
#include "shellmemory.h"

int parseInput(char ui[]);

// Hi my name is Seb
//  another one

// Start of everything
int main(int argc, char *argv[])
{
    printf("Shell version 1.3 created September 2024\n");

    // help();
    printf("\n");

    int isBackground = 0;

    char prompt = '$';              // Shell prompt
    char userInput[MAX_USER_INPUT]; // user's input stored here
    int errorCode = 0;              // zero means no error, default

    for (int i = 0; i < MAX_USER_INPUT; i++)
    {
        userInput[i] = '\0';
    }

    FILE *input = stdin;
    int isBatch = 0;
    if (!isatty(fileno(stdin)))
    {
        isBatch = 1;
    }
    struct Script *newScript = NULL;
    char *hashtag_line = NULL;


    // init shell memory
    mem_init();

    while (1)
    {
        if (!isBatch)
        { 
            printf("%c ", prompt);
        }

        if (fgets(userInput, MAX_USER_INPUT - 1, input) == NULL)
        {
            if (feof(stdin))
            {
                clearerr(stdin);
                fclose(input);
                input = stdin;
                prompt = '$';
                // printf("Entering interactive mode\n");
                continue;
            }
            break;
        }
        // printf ("Got here1. line: %s\n", userInput);
        if (strrchr(userInput, '#') != NULL)
        {
            // printf ("Got here2. line: %s\n", userInput);

            hashtag_line = strdup(userInput);
            isBackground = 1;
            newScript = create_script(script_count, NULL);
            // printf ("Got here3. line: %s\n", hashtag_line);

        }
        else if (isBackground == 1)
        {
            // printf("Got into adding to script\n");
            add_line_to_script(newScript, userInput);
            // printf("leaving adding to script\n");
            
        }
        else {
        errorCode = parseInput(userInput);
        }
        if (errorCode == -1)
            exit(99); // ignore all other errors
        memset(userInput, 0, sizeof(userInput));

        // printf("Hashtag_line: %s\n", hashtag_line);
    }
    struct PCB *script_pcb = create_pcb(script_count, newScript->current);
    
    if (isBackground == 1){
        //loadProcessestoMemory(newScript);
        // printf("Got here. Hashtag_line: %s. Script_count: %d\n", hashtag_line, script_count);
        errorCode = parseInput(hashtag_line);
    }

    return 0;
}

int wordEnding(char c)
{
    // You may want to add ';' to this at some point,
    // or you may want to find a different way to implement chains.
    return c == '\0' || c == '\n' || c == ' ';
}

int parseInput(char inp[])
{
    //printf("Here\n");
    int errorCode;

    char *commands[10];
    int commandCount = 0;

    char *token = strtok(inp, ";");

    while (token != NULL && commandCount != 10)
    {
        commands[commandCount] = token;
        commandCount++;
        token = strtok(NULL, ";");
    }

    for (int commandNUM = 0; commandNUM < commandCount; commandNUM++)
    {
        //printf("Here\n");
        char *instruction = commands[commandNUM];
        char tmp[200], *words[100];
        int ix = 0, w = 0; // ix = index, w = word
        int wordlen;

        for (ix = 0; instruction[ix] == ' ' && ix < 1000; instruction++)
            ; // skip white spaces
        while (instruction[ix] != '\n' && instruction[ix] != '\0' && ix < 1000)
        {
            // extract a word
            for (wordlen = 0; !wordEnding(instruction[ix]) && ix < 1000; ix++, wordlen++)
            {
                tmp[wordlen] = instruction[ix];
            }
            tmp[wordlen] = '\0';
            words[w] = strdup(tmp);
            w++;
            if (instruction[ix] == '\0')
                break;
            ix++;
        }
        //printf("Here\n");
        //printf("words: %s, w:%d\n", *words, w);
        errorCode = interpreter(words, w);
        //printf("Here\n");
    }
    
    return errorCode;
}
