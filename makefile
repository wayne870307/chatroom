all: server.c client.c
	gcc -pthread server.c -o server
	gcc -pthread client.c -o client
