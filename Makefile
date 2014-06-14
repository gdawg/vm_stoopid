CC=gcc
CFLAGS=-Wall
SOURCES=vm_stoop.c
EXECUTABLE=vm_stoop

all:
	$(CC) $(CFLAGS) $(SOURCES) -o $(EXECUTABLE)
clean:
	rm $(EXECUTABLE)
