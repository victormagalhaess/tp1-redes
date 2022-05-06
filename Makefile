CC=gcc # compilador, troque para gcc se preferir utilizar C
CFLAGS=-Wall -Wextra -g # compiler flags, troque o que quiser, exceto bibliotecas externas
EXEC_CLIENT=./client # nome do executavel que sera gerado, nao troque
EXEC_SERVER=./server

all: $(EXEC_CLIENT) $(EXEC_SERVER)

$(EXEC_CLIENT): client.c common.o
	$(CC) $(CFLAGS) client.c common.o -o $(EXEC_CLIENT)

$(EXEC_SERVER): server.c common.o
	$(CC) $(CFLAGS) server.c common.o -o $(EXEC_SERVER)

common.o: common.c
	$(CC) $(CFLAGS) -c common.c -o common.o



