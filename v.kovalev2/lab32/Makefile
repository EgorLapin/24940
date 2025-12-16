CC      = gcc
CFLAGS  = -Wall -Wextra -O2

TARGETS = server client
SRC     = server.c client.c
OBJ     = $(SRC:.c=.o)

all: $(TARGETS)

server: server.o
	$(CC) $(CFLAGS) -o $@ $^

client: client.o
	$(CC) $(CFLAGS) -o $@ $^

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f $(OBJ) $(TARGETS)
