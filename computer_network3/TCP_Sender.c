#include <stdio.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <unistd.h>
#include <string.h>
#include <time.h>
#include <netinet/in.h>
#include <netinet/tcp.h>

#define BUFFER_SIZE 65536

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
    char ans[4] = "yes";
    struct sockaddr_in server;
    char buffer[BUFFER_SIZE] = {0};
    memset(&server, 0, sizeof(server));
    int big_size = 2097152;
    int sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    char *message = util_generate_random_data(big_size);

    if (sock < 0)
    {
        perror("socket(2)");
        return 1;
    }
    printf("socket was good \n");
    if (strcmp(argv[6], "reno") == 0)
    {
        // set to be reno
        setsockopt(sock, IPPROTO_TCP, TCP_CONGESTION, "reno", strlen("reno"));
    }
    else if (strcmp(argv[6], "cubic") == 0)
    {
        // set to be cubic
        setsockopt(sock, IPPROTO_TCP, TCP_CONGESTION, "cubic", strlen("cubic"));
    }
    else
    {
        printf("Invalid TCP congestion control algorithm\n");
        return -1;
    }

    if (inet_pton(AF_INET, argv[2], &server.sin_addr) <= 0)
    {
        perror("inet_pton(3)");
        close(sock);
        return 1;
    }

    server.sin_family = AF_INET;
    server.sin_port = htons(atoi(argv[4]));

    int con = connect(sock, (struct sockaddr *)&server, sizeof(server));
    printf("the con is %d\n", con);
    if (con < 0)
    {
        printf("badd\n");
        perror("connect(2)");
        close(sock);
        return 1;
    }

    int bytesSent = 0;

    while (1)
    {
        bytesSent = 0;
        // puts("int the sender loop");

        while (bytesSent < big_size)
        {
            int cur_sent = send(sock, message + bytesSent, big_size - bytesSent, 0);
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
        printf("message was successfully sent .\n");
        printf("message size was %d\n", bytesSent);

        printf("resend?");
        scanf("%s", ans);
        if (ans[0] != 'y')
        {
            break;
        }
        // int bytes_sent = send(sock, message, strlen(message) + 1, 0);
        //  if (bytes_sent == -1)
        //  {
        //      perror("send() failed");
        //  }
        //  else if (bytes_sent == 0)
        //  {
        //      printf("peer has closed the TCP connection prior to send().\n");
        //  }
        //  else if (strlen(message) + 1 > bytes_sent)
        //  {
        //      printf("sent only %d bytes from the required %d.\n", strlen(message) + 1, bytes_sent);
        //  }
        //  else
        //  {
        //      printf("message was successfully sent .\n");
        //  }
    }
    free(message);
    char *exit = "exit";
    bytesSent = send(sock, exit, strlen(exit) + 1, 0);
    printf("%d\n", bytesSent);
    close(sock);
    return 0;
}
