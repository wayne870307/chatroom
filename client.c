#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <pthread.h>
#include <string.h>
#include <unistd.h>
#include <sys/stat.h> 
#include <fcntl.h>
#include <sys/sendfile.h>
#include <stdlib.h>

#define BUFSIZE 1024

int server_fd, control = 0;
char op = '\0';

void *read_other(void *);

int main(int argc, char *argv[]) 
{
    struct sockaddr_in server_addr;
    pthread_t          tid;
    char outputBuf[BUFSIZE], name[20];

    server_fd = socket(AF_INET, SOCK_STREAM, 0);

    bzero(&server_addr, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(8080);
    inet_pton(AF_INET, "127.0.0.1", &server_addr.sin_addr);

    connect(server_fd, (struct sockaddr *) &server_addr, sizeof(server_addr));

    printf("Please insert your name: ");
    fgets(name, sizeof(name), stdin);
    name[strlen(name) - 1] = '\0';
    write(server_fd, name, strlen(name));

    pthread_create(&tid, NULL, read_other, NULL);

    while(1) 
	{
        fgets(outputBuf, BUFSIZE, stdin);
        outputBuf[strlen(outputBuf) - 1] = '\0';
        if (control == 1) 
		{
            if (!strcmp(outputBuf, "@yes")) 
			{
                op = 'y';
                continue;
            }
            else 
			{
                op = 'n';
                continue;
            }
        }
        write(server_fd ,outputBuf, strlen(outputBuf)); 
		
        if (!strcmp(outputBuf, "@quit"))
		{
            return 0;
		}
        else if (!strncmp(outputBuf, "@send", 5))
		{
            char *fileName = strrchr(outputBuf, ' ') + 1;
            int file_fd = open(fileName, O_RDONLY);
            sendfile(server_fd, file_fd, NULL, BUFSIZE);
            close(file_fd); 
        }
    }
	return 0;
}
 
void *read_other(void *arg) 
{
    char inputBuf[BUFSIZE];
    while(1) 
	{
        bzero(inputBuf, sizeof(inputBuf));
        read(server_fd, inputBuf, sizeof(inputBuf));
        if (!strncmp(inputBuf, "[From", 5)) 
		{
            printf("%s", inputBuf);
            fprintf(stderr, "Do you want to receive? (@yes or @no) ");
            control = 1;
            while (!op);
            if (op == 'y') 
			{
                char *fileName = strrchr(inputBuf, ']') + 1;
                fileName[strlen(fileName) - 1] = '\0';
                int file_fd = open(fileName, O_WRONLY | O_CREAT, 0777);
                bzero(inputBuf, sizeof(inputBuf));
                read(server_fd, inputBuf, sizeof(inputBuf));
                write(file_fd, inputBuf, sizeof(inputBuf));
                close(file_fd);
            }
            else 
			{
                bzero(inputBuf, sizeof(inputBuf));
                read(server_fd, inputBuf, sizeof(inputBuf));
            }
            control = 0;
            op = '\0';
        }
        else
		{
            printf("%s", inputBuf);
		}
    }
}