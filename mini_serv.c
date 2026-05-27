#include <unistd.h>
#include <stdlib.h>
#include <sys/socket.h> // To use socket(), bind(), listen()
#include <netinet/in.h> // For sockaddr_in
#include <string.h>     // For memset()
#include <poll.h>
#include <errno.h>
#include <string.h>
#include <netdb.h>

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
        for (int i = 0; i < nfds; ++i)
        {
            if (pfds[i].revents & POLLIN)
            {
                if (pfds[i].fd == sockfd)
                {
                    //new connection
                    //accept() the connection → get client_fd
                    // Add to pfds array
                    // Add to clients array
                    // Broadcast "server: client %d just arrived\n" to all existing clients
                    int client_fd = accept()
                }
                else
                {
                    //client msg or disconnection
                }          
            }
        }
    }

    return 0;
}
