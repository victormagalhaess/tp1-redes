CC=gcc 
CFLAGS=-Wall -Wextra -g 
EXEC_CLIENT=./client 
EXEC_SERVER=./server

all: $(EXEC_CLIENT) $(EXEC_SERVER)

$(EXEC_CLIENT): client.c common.o
	$(CC) $(CFLAGS) client.c common.o -o $(EXEC_CLIENT)

$(EXEC_SERVER): server.c common.o
	$(CC) $(CFLAGS) server.c common.o -o $(EXEC_SERVER)

common.o: common.c
	$(CC) $(CFLAGS) -c common.c -o common.o

clean:
	rm -rf *.o server client