CC=gcc
CFLAGS=-Wall -Wextra -O2 -pthread

SRC=src/producer.c src/metrics.c
OBJ=$(SRC:.c=.o)

producer: $(OBJ)
	$(CC) $(CFLAGS) -o producer $(OBJ)

clean:
	rm -f src/*.o producer