#include <unistd.h>
#include <stdlib.h>
#include <sys/socket.h> // To use socket(), bind(), listen()
#include <netinet/in.h> // For sockaddr_in
#include <string.h>     // For memset()
#include <poll.h>

int main(int argc, char *argv[])
{
    if (argc != 2)
    {
        write(STDERR_FILENO, "Wrong number of arguments\n", 25);
        return 1;
    }
    int port = atoi(argv[1]);
    return 0;
}
