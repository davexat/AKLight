CC=gcc
CFLAGS=-Wall -Wextra -O2 -pthread

SRC=src/consumer.c
OBJ=$(SRC:.c=.o)

consumer: $(OBJ)
	$(CC) $(CFLAGS) -o consumer $(OBJ)

clean:
	rm -f src/*.o consumer
