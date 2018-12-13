#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <pthread.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/stat.h> 
#include <fcntl.h>

#define MAXCLI 128
#define BUFSIZE 1024

int client_fd[MAXCLI], n = 0;
struct sockaddr_in client_addr[MAXCLI];
socklen_t client_len = sizeof(client_addr[0]);
char name[MAXCLI][20];

void *chatroom(void *);

int main(int argc, char *argv[]) 
{
    int                server_fd;
    struct sockaddr_in server_addr;
    pthread_t          tid;

    server_fd = socket(AF_INET, SOCK_STREAM, 0);

    bzero(&server_addr, sizeof(server_addr));
    server_addr.sin_family      = AF_INET;
    server_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    server_addr.sin_port        = htons(8080);

    bind(server_fd, (struct sockaddr *) &server_addr, sizeof(server_addr));
    listen(server_fd, MAXCLI);

    int index = 0;
    while(1) 
	{
        client_fd[n] = accept(server_fd, (struct sockaddr *) &client_addr[n], &client_len);
        index = n;
        ++n;
        pthread_create(&tid, NULL, chatroom, &index);
    }
	return 0;
}
 
void *chatroom(void *index) 
{
    int i = *(int *)index;
    char inputBuf[BUFSIZE], outputBuf[BUFSIZE];

    read(client_fd[i], name[i], sizeof(name[i]));

    while(1) 
	{
        bzero(inputBuf, sizeof(inputBuf));
        read(client_fd[i], inputBuf, sizeof(inputBuf));

        if (!strcmp(inputBuf, "@quit")) 
		{
            client_fd[i] = 0;
            pthread_exit(0);
        }
        else if (!strcmp(inputBuf, "@who")) 
		{
            bzero(outputBuf, sizeof(outputBuf));
            for (int j = 0; j < n; ++j)
                if (client_fd[j]) 
				{
                    strcat(outputBuf, name[j]);
                    strcat(outputBuf, "\n");
                }
            write(client_fd[i], outputBuf, sizeof(outputBuf));
        }
        else if (!strncmp(inputBuf, "@send", 5)) 
		{
            int to_fd;
            char *fileName = strrchr(inputBuf, ' ') + 1;
            char *to = strchr(inputBuf, ' ') + 1;
            to[strchr(to, ' ') - to] = '\0';
            sprintf(outputBuf, "[From %s]%s\n", name[i], fileName);
            for (int j = 0; j < n; ++j)
                if (!strcmp(name[j], to) && client_fd[j]) 
				{
                    write(client_fd[j], outputBuf, sizeof(outputBuf));
                    to_fd = client_fd[j];
                    break;
                }
            bzero(inputBuf, sizeof(inputBuf));
            read(client_fd[i], inputBuf, sizeof(inputBuf));
            write(to_fd, inputBuf, sizeof(inputBuf));
        }
        else if (!strncmp(inputBuf, "@direct", 7)) 
		{
			char *to = strchr(inputBuf, ' ') + 1;
            char *message = strchr(to, ' ') + 1;
            *(message - 1) = '\0';
            sprintf(outputBuf, "[Direct Message]%s: %s\n", name[i], message);
            for (int j = 0; j < n; ++j)
                if (!strcmp(name[j], to) && client_fd[j]) 
				{
                    write(client_fd[j], outputBuf, sizeof(outputBuf));
                    break;
                }
        }
        else 
		{
            sprintf(outputBuf, "%s: %s\n", name[i],inputBuf);
            for (int j = 0; j < n; ++j)
			{
                if (client_fd[j])
				{
                    write(client_fd[j], outputBuf, sizeof(outputBuf));
				}
			}
        }
    }
}