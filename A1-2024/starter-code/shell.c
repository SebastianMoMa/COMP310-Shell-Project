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

            //help();
            printf("\n");



            char prompt = '$';  				// Shell prompt
            char userInput[MAX_USER_INPUT];		// user's input stored here
            int errorCode = 0;					// zero means no error, default
        
            for (int i = 0; i < MAX_USER_INPUT; i++) {
                userInput[i] = '\0';
            }

            
            FILE *input = stdin; 
            int isBatch = 0;
            if (!isatty(fileno(stdin))) { 
                isBatch = 1;
            }
        
            //init shell memory
            mem_init();
            while(1) {		
                if (!isBatch){	  // Problem lies here for the batch mode, or the way I handle the prompt
                    printf("%c ", prompt);}
            
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
            int errorCode;
        
            char *commands[10];
            int commandCount = 0;
            
            char *token = strtok(inp, ";");

            while (token != NULL && commandCount !=10){
            commands[commandCount] = token;
            commandCount ++;
            token = strtok(NULL, ";");
            }
            
            for (int commandNUM = 0; commandNUM < commandCount; commandNUM++)
            {
            char *instruction = commands[commandNUM];
            char tmp[200], *words[100];                            
            int ix = 0, w = 0; //ix = index, w = word
            int wordlen;
            
            for (ix = 0; instruction[ix] == ' ' && ix < 1000; instruction++); // skip white spaces
            while (instruction[ix] != '\n' && instruction[ix] != '\0' && ix < 1000) {
                // extract a word
                for (wordlen = 0; !wordEnding(instruction[ix]) && ix < 1000; ix++, wordlen++) {
                    tmp[wordlen] = instruction[ix];                        
                }
                tmp[wordlen] = '\0';
                words[w] = strdup(tmp);
                w++;
                if (instruction[ix] == '\0') break;
                ix++; 
            }
            errorCode = interpreter(words, w);
            }
            return errorCode;
        }


