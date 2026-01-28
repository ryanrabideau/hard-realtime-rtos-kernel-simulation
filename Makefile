# Makefile for Minimal Hard Real-Time OS Kernel Simulation

CC = gcc
CFLAGS = -std=c11 -Wall -Wextra -Iinclude

SOURCES = main.c kernel/kernel.c kernel/task.c
OBJECTS = $(SOURCES:.c=.o)
TARGET = minimal-rtos

all: $(TARGET)

$(TARGET): $(OBJECTS)
	$(CC) $(OBJECTS) -o $@

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f $(OBJECTS) $(TARGET) output.txt scenario_*.txt *.o

run: $(TARGET)
	./$(TARGET)

.PHONY: all clean run
