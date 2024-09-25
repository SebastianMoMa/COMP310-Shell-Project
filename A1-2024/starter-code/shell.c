#include <stdio.h>
#include <stdlib.h>
#include <string.h> 
#include <unistd.h>
#include "shell.h"
#include "interpreter.h"
#include "shellmemory.h"

int parseInput(char ui[]);

//Hi my name is Seb
// another one

// Start of everything
int main(int argc, char *argv[]) {
    printf("Shell version 1.3 created September 2024\n");
        //printf("got here\n");

    help();
    //printf("got here\n");
    //fflush(stdout);


    char prompt = '$';  				// Shell prompt
    char userInput[MAX_USER_INPUT];		// user's input stored here
    int errorCode = 0;					// zero means no error, default
    //printf("got here\n");
    //init user input
    for (int i = 0; i < MAX_USER_INPUT; i++) {
        userInput[i] = '\0';
    }

    
    FILE *input = stdin; //Essentially like saying that it should be interactive as default
    //printf("this is arg_count: %i", argc);
    int isBatch = 0;
    if (!isatty(fileno(stdin))) { 
        //prompt = "\0"; 
         isBatch = 1;
    }
   
    //init shell memory
    mem_init();
    while(1) {		
        if (!isBatch){	  // Problem lies here for the batch mode, or the way I handle the prompt
            printf("%c ", prompt);}
        // here you should check the unistd library 
        // so that you can find a way to not display $ in the batch mode
    if (fgets(userInput, MAX_USER_INPUT - 1, input) == NULL) {
        if (feof(stdin)) {
                clearerr(stdin);
                fclose(input);
                input = stdin;
                prompt = '$';
                //printf("Entering interactive mode\n");
                continue;
            }
            break; 
        }
    
/*
            if (isBatch) {
                fclose(input); // Close the batch file
                printf("closed file\n");
                isBatch = 0;
                input = stdin;
            }
            input = stdin;
            prompt = '$';
            printf("Entering interactive mode\n");

            continue; 
        }
*/
        errorCode = parseInput(userInput);
        if (errorCode == -1) exit(99);	// ignore all other errors
        memset(userInput, 0, sizeof(userInput));
    }

    return 0;
}

int wordEnding(char c) {
    // You may want to add ';' to this at some point,
    // or you may want to find a different way to implement chains.
    return c == '\0' || c == '\n' || c == ' ';
}

int parseInput(char inp[]) {
    char tmp[200], *words[100];                            
    int ix = 0, w = 0;
    int wordlen;
    int errorCode;
    for (ix = 0; inp[ix] == ' ' && ix < 1000; ix++); // skip white spaces
    while (inp[ix] != '\n' && inp[ix] != '\0' && ix < 1000) {
        // extract a word
        for (wordlen = 0; !wordEnding(inp[ix]) && ix < 1000; ix++, wordlen++) {
            tmp[wordlen] = inp[ix];                        
        }
        tmp[wordlen] = '\0';
        words[w] = strdup(tmp);
        w++;
        if (inp[ix] == '\0') break;
        ix++; 
    }
    errorCode = interpreter(words, w);
    return errorCode;
}


