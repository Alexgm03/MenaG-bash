CC = gcc
CFLAGS = -Wall -Wextra -std=c11
TARGET = MenaG-bash
SRC = MenaG-bash.c

.PHONY: all run clean

all: $(TARGET)

$(TARGET): $(SRC)
	$(CC) $(CFLAGS) -o $(TARGET) $(SRC)

run: all
	./$(TARGET)

clean:
	rm -f $(TARGET) *.o

