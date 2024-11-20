# Compiler and Flags
CC = gcc
CFLAGS = #-Wall -g

# Default values for framesize and varmemsize (can be overridden)
framesize ?= 6
varmemsize ?= 10

# Target for the executable
mysh: shell.o interpreter.o shellmemory.o
	$(CC) $(CFLAGS) -D FRAMESIZE=$(framesize) -D VARMEMSIZE=$(varmemsize) -o mysh shell.o interpreter.o shellmemory.o

# How to build shell.o
shell.o: shell.c
	$(CC) $(CFLAGS) -D FRAMESIZE=$(framesize) -D VARMEMSIZE=$(varmemsize) -c shell.c

# How to build interpreter.o
interpreter.o: interpreter.c
	$(CC) $(CFLAGS) -D FRAMESIZE=$(framesize) -D VARMEMSIZE=$(varmemsize) -c interpreter.c

# How to build shellmemory.o
shellmemory.o: shellmemory.c
	$(CC) $(CFLAGS) -D FRAMESIZE=$(framesize) -D VARMEMSIZE=$(varmemsize) -c shellmemory.c

# Clean up intermediate files
clean:
	rm -f mysh *.o