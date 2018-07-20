CC = gcc
CFLAGS = -pedantic -std=gnu99 -Wall
DEBUG_FLAGS = -g $(CFLAGS)

ping_client: main.c flags.h flags.c ping_client.h ping_client.c stats.h stats.c
	$(CC) $(DEBUG_FLAGS) flags.c ping_client.c stats.c main.c -o ping_client -lpthread

.PHONY: clean
clean:
	rm -rf *.o *.dSYM ping_client
