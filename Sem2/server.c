#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <arpa/inet.h>

#define DEFAULT_PORT htons (8080)
#define DEFAULT_IP htonl (INADDR_LOOPBACK)
#define QUEUE_MAX_LEN 128
#define BUFFER_SIZE 1024

typedef enum
{
    TCP = 0,
    UDP = 1
} Protocol;

int StartServer (Protocol prt);

void ProcessTCP (int sockfd);
void ProcessUDP (int sockfd);

void PrintMessage (char* buf, struct sockaddr_in* addr);

int main (int argc, char** argv)
{
    int sockfd = 0;

    if (argc == 1 || strcmp (argv[1], "-UDP"))
    {
        if ((sockfd = StartServer (TCP)) == -1)
            return 0;
        ProcessTCP (sockfd); 
    }

    else
    {
        if ((sockfd = StartServer (UDP)) == -1)
            return 0;
        ProcessUDP (sockfd);
    }

    close (sockfd);
    return 0;
}

int StartServer (Protocol prt)
{
    #define try(cond, str)  if (cond)           \
                            {                   \
                                close (sockfd); \
                                perror (str);   \
                                return -1;      \
                            }

    int sockfd = socket (AF_INET, (prt == TCP) ? SOCK_STREAM : SOCK_DGRAM, 0);
    try (sockfd == -1, "Socket error");

    struct sockaddr_in addr = {AF_INET, DEFAULT_PORT, {DEFAULT_IP}, {}};
    try (bind (sockfd, (struct sockaddr*) &addr, sizeof (struct sockaddr_in)) == -1, "Bind error");

    int reuse_on = 1;
    try (setsockopt (sockfd, SOL_SOCKET, SO_REUSEADDR, &reuse_on, sizeof (reuse_on)) == -1, "Setsockopt error")

    if (prt == TCP)
    {
        try (listen (sockfd, QUEUE_MAX_LEN) == -1, "Listen error");
    }

    #undef try
    return sockfd;
}

void ProcessTCP (int sockfd)
{
    (void) sockfd;
}

void ProcessUDP (int sockfd)
{
    struct sockaddr_in addr = {};
    socklen_t addrlen = sizeof (addr);
    char buf[BUFFER_SIZE] = "";

    while (1)
    {
        recvfrom (sockfd, buf, BUFFER_SIZE - 1, 0, (struct sockaddr*) &addr, &addrlen);
        PrintMessage (buf, &addr);
    }
}

void PrintMessage (char* buf, struct sockaddr_in* addr)
{
    char* ip = inet_ntoa (addr->sin_addr);
    int port = ntohs (addr->sin_port);

    printf ("[%s:%d]:%s\n", ip, port, buf);
    return;
}
