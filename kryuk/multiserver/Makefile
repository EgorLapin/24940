CC = gcc
CFLAGS = -Wall -Wextra -std=c99
LDFLAGS = -lsocket -lnsl
TARGET1 = server
SOURCE1 = server.c
TARGET2 = client
SOURCE2 = client.c

all: $(TARGET1) $(TARGET2)

$(TARGET1): $(SOURCE1)
	$(CC) $(CFLAGS) -o $(TARGET1) $(SOURCE1) $(LDFLAGS)

$(TARGET2): $(SOURCE2)
	$(CC) $(CFLAGS) -o $(TARGET2) $(SOURCE2) $(LDFLAGS)

.PHONY: all