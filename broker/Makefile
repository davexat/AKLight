CC=gcc
CFLAGS=-Wall -Wextra -O2 -pthread

SRC=src/broker.c
OBJ=$(SRC:.c=.o)

broker: $(OBJ)
	$(CC) $(CFLAGS) -o broker $(OBJ)

clean:
	rm -f src/*.o broker