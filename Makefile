# Makefile for Milestone 1
CC = gcc
CFLAGS = -Wall -Wextra -std=c11

TARGET = VMCacheSim.exe
SRC = main.c

all: $(TARGET)

$(TARGET): $(SRC)
	$(CC) $(CFLAGS) -o $(TARGET) $(SRC)

clean:
	rm -f $(TARGET)