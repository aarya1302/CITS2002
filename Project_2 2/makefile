# Compiler and flags
CC = gcc
CFLAGS = -Wall -g

# Object files
OBJS = directories.o files.o glob2regex.o globals.o hashstring.o hashtable.o list.o mysync.o patterns.o

# Target executable
TARGET = mysync

# Rules
all: $(TARGET)

$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) -o $@ $^

%.o: %.c mysync.h
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f $(OBJS) $(TARGET)
