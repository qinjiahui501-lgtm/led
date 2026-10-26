CC = gcc
CFLAGS = -Wall -g
TARGET = ledctl

all: $(TARGET)

$(TARGET): ledctl.c
	$(CC) $(CFLAGS) -o $(TARGET) ledctl.c
clean:
	rm -f $(TARGET) *.o