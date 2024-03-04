#include <stdio.h>
#include <stdlib.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <unistd.h>
#include <string.h>
#include <time.h>
#include <netinet/in.h>
#include <netinet/tcp.h>

#define IP_ARG 2
#define PORT_ARG 4
#define ALGO_ARG 6
#define FILE_SIZE 2097152

// make random data
char *util_generate_random_data(unsigned int size)
{
    char *buffer = NULL;
    // Argument check.
    if (size == 0)
    {
        return NULL;
    }
    buffer = (char *)calloc(size, sizeof(char));
    // Error checking.
    if (buffer == NULL)
    {
        return NULL;
    }
    srand(time(NULL));
    for (unsigned int i = 0; i < size; i++)
    {
        *(buffer + i) = ((unsigned int)rand() % 256);
    }
    return buffer;
}

int main(int argc, char *argv[])
{
    char ans[PORT_ARG] = "yes";
    struct sockaddr_in server;
    memset(&server, 0, sizeof(server));
    int sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    char *message = util_generate_random_data(FILE_SIZE);

    printf("Starting Sender.\n");
    if (sock < 0)
    {
        perror("socket(2)");
        return 1;
    }
    if (strcmp(argv[ALGO_ARG], "reno") == 0)
    {
        // set to be reno
        setsockopt(sock, IPPROTO_TCP, TCP_CONGESTION, "reno", strlen("reno"));
    }
    else if (strcmp(argv[ALGO_ARG], "cubic") == 0)
    {
        // set to be cubic
        setsockopt(sock, IPPROTO_TCP, TCP_CONGESTION, "cubic", strlen("cubic"));
    }
    else
    {
        printf("Invalid TCP congestion control algorithm.\n");
        return -1;
    }

    if (inet_pton(AF_INET, argv[IP_ARG], &server.sin_addr) <= 0)
    {
        perror("inet_pton(3)");
        close(sock);
        return 1;
    }

    server.sin_family = AF_INET;
    server.sin_port = htons(atoi(argv[PORT_ARG]));

    printf("Connecting to Reciever...\n");
    int con = connect(sock, (struct sockaddr *)&server, sizeof(server));
    if (con < 0)
    {
        perror("connect(2)");
        close(sock);
        return 1;
    }

    int bytesSent = 0;

    while (1)
    {
        bytesSent = 0;

        printf("Reciever connected, beginning to send file...\n");

        while (bytesSent < FILE_SIZE)
        {
            int cur_sent = send(sock, message + bytesSent, FILE_SIZE - bytesSent, 0);
            bytesSent += cur_sent;
            if (cur_sent < 0)
            {
                perror("send(2)");
                close(sock);
                return 1;
            }
            else if (cur_sent == 0)
            {
                printf("peer has closed the TCP connection prior to send().\n");
                close(sock);
                return 1;
            }
        }
        printf("File was successfully sent.\n");

        char buff[2];
        recv(sock, buff, sizeof(buff), 0);

        printf("Do you want to resend the file? [yes/no]: ");
        scanf("%s", ans);
        if (ans[0] != 'y')
        {
            break;
        }

        send(sock, ans, strlen(ans) + 1, 0);
        recv(sock, buff, sizeof(buff), 0);
    }

    free(message);
    char *exit = "exit";
    bytesSent = send(sock, exit, strlen(exit) + 1, 0);
    close(sock);

    printf("Sender end.\n");

    return 0;
}
