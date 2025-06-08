CC = gcc
CFLAGS = -Wall -Wextra -std=c99 -g -lm

TARGET = simulador
SOURCES = simulador.c memoria.c algoritmos.c pagetable.c
OBJECTS = $(SOURCES:.c=.o)
HEADERS = memoria.h algoritmos.h pagetable.h

all: $(TARGET)

$(TARGET): $(OBJECTS)
	$(CC) $(CFLAGS) -o $(TARGET) $(OBJECTS)

%.o: %.c $(HEADERS)
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f $(OBJECTS) $(TARGET)