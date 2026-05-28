#include <errno.h>
#include <unistd.h>
#include <string.h>     // For memset()
#include <sys/socket.h> // To use socket(), bind(), listen()
#include <netinet/in.h> // For sockaddr_in
#include <netdb.h>
#include <poll.h>
#include <stdlib.h>
#include <stdio.h>

int extract_message(char **buf, char **msg)
{
	char	*newbuf;
	int	i;

	*msg = 0;
	if (*buf == 0)
		return (0);
	i = 0;
	while ((*buf)[i])
	{
		if ((*buf)[i] == '\n')
		{
			newbuf = calloc(1, sizeof(*newbuf) * (strlen(*buf + i + 1) + 1));
			if (newbuf == 0)
				return (-1);
			strcpy(newbuf, *buf + i + 1);
			*msg = *buf;
			(*msg)[i + 1] = 0;
			*buf = newbuf;
			return (1);
		}
		i++;
	}
	return (0);
}

char *str_join(char *buf, char *add)
{
	char	*newbuf;
	int		len;

	if (buf == 0)
		len = 0;
	else
		len = strlen(buf);
	newbuf = malloc(sizeof(*newbuf) * (len + strlen(add) + 1));
	if (newbuf == 0)
		return (0);
	newbuf[0] = 0;
	if (buf != 0)
		strcat(newbuf, buf);
	free(buf);
	strcat(newbuf, add);
	return (newbuf);
}

void fatal_error(void)
{
    write(STDERR_FILENO, "Fatal error\n", 12);
    exit(1);
}

typedef struct s_client
{
    int     fd;
    int     id;
    char    *buf;
}   t_client;

int main(int argc, char *argv[])
{
    if (argc != 2)
    {
        char msg[] = "Wrong number of arguments\n";
        write(STDERR_FILENO, msg, strlen(msg));
        return 1;
    }
    int port = atoi(argv[1]);

    int sockfd;
	struct sockaddr_in servaddr; 

	// socket create and verification 
	sockfd = socket(AF_INET, SOCK_STREAM, 0); 
	if (sockfd == -1)
    {
        fatal_error();
	} 
	bzero(&servaddr, sizeof(servaddr)); 

	// assign IP, PORT 
	servaddr.sin_family = AF_INET; 
	servaddr.sin_addr.s_addr = htonl(2130706433); //127.0.0.1
	servaddr.sin_port = htons(port); 
  
	// Binding newly created socket to given IP and verification 
	if ((bind(sockfd, (const struct sockaddr *)&servaddr, sizeof(servaddr))) != 0)
    {
        fatal_error();
	}

	if (listen(sockfd, 10) != 0)
    {
        fatal_error();
	}

    t_client        clients[1024];  // indexed by fd
    struct pollfd   pfds[1024];     // packed array for poll()
    int             nfds = 0;       // current count
    int             next_id = 0;    // client id counter

    bzero(clients, sizeof(clients));
    bzero(pfds, sizeof(pfds));

    //setup 1st server socket
    pfds[0].fd = sockfd;
    pfds[0].events = POLLIN;
    nfds++;

    while (1)
    {
        if (poll(pfds, nfds, -1) < 0)
        {
            fatal_error();
        }
        for (int i = 0; i < nfds; ++i) // check all fds for events
        {
            if (pfds[i].revents & POLLIN) // if this fd has event
            {
                if (pfds[i].fd == sockfd) // if event on server socket
                {
                    //new connection
                    //accept() the connection → get client_fd
                    // Add to pfds array
                    // Add to clients array
                    // Broadcast "server: client %d just arrived\n" to all existing clients
                    int client_fd = accept(sockfd, NULL, NULL);
                    if (client_fd < 0)
                    {
                        fatal_error();
                    }
                    pfds[nfds].fd = client_fd;
                    pfds[nfds++].events = POLLIN;

                    clients[client_fd].fd = client_fd;
                    clients[client_fd].id = next_id++;
                    clients[client_fd].buf = NULL;

                    char msg[128];
                    sprintf(msg, "server: client %d just arrived\n", clients[client_fd].id);
                    for (int j = 0; j < nfds; ++j)
                    {
                        if (pfds[j].fd != client_fd && pfds[j].fd != sockfd)
                        {
                            send(pfds[j].fd, msg, strlen(msg), 0);
                        }
                    }
                }
                else // if event on client socket
                {
                    int client_fd = pfds[i].fd;
                    char buffer[1024];
                    int bytes = recv(client_fd, buffer, sizeof(buffer) - 1, 0);
                    if (bytes > 0) // client message
                    {
                        buffer[bytes] = '\0';  // null terminate
                        clients[client_fd].buf = str_join(clients[client_fd].buf, buffer);
                        char *msg;
                        while (extract_message(&clients[client_fd].buf, &msg))
                        {
                            for (int j = 0; j < nfds; ++j)
                            {
                                if (pfds[j].fd != client_fd && pfds[j].fd != sockfd)
                                {
                                    char prefix[64];
                                    sprintf(prefix, "client %d: ", clients[client_fd].id);
                                    send(pfds[j].fd, prefix, strlen(prefix), 0);
                                    send(pfds[j].fd, msg, strlen(msg), 0);
                                }
                            }
                            free(msg);
                        }
                    }
                    else // client disconnected or error
                    {
                        // disconnect
                        char msg[128];
                        sprintf(msg, "server: client %d just left\n", clients[client_fd].id);
                        for (int j = 0; j < nfds; ++j)
                        {
                            if (pfds[j].fd != client_fd && pfds[j].fd != sockfd)
                            {
                                send(pfds[j].fd, msg, strlen(msg), 0);
                            }
                        }
                        free(clients[client_fd].buf);
                        bzero(&clients[client_fd], sizeof(t_client));// size of the struct, not the pointer
                        pfds[i] = pfds[nfds - 1];
                        nfds--;
                        i--;
                        close(client_fd);
                    }
                }
            }
            // else: no events, do nothing         
        }
    }
    
    return 0;
}
