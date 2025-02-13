CC = gcc
CFLAGS = -Wall -Wextra
INCLUDES = -Iinclude
SRC = src/job_control.c src/main.c src/parser.c src/process.c src/signals.c

all:
	$(CC) $(CFLAGS) $(INCLUDES) -o yash $(SRC)

clean:
	rm -f yash
