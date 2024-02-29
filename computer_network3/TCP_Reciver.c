#include <stdio.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <unistd.h>
#include <string.h>
#include <time.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include "List.c"

#define MAX_CLIENTS 1
#define BUFFER_SIZE 2097152
#define DEV 1000
#define IP "127.0.0.1"

int main(int argc, char *argv[])
{
    List *dataList = List_alloc();
    struct sockaddr_in server;
    struct sockaddr_in client;
    socklen_t client_len = sizeof(client);
    char *message = "";
    int messageLen = strlen(message) + 1;
    clock_t start_t, end_t;
    double total_t;

    int opt = 1;
    memset(&server, 0, sizeof(server));
    memset(&client, 0, sizeof(client));
    int sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

    printf("Starting Receiver.\n");
    if (sock == -1)
    {
        perror("socket(2)");
        return 1;
    }
    if (setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0)
    {
        perror("setsockopt(2)");
        close(sock);
        return 1;
    }

    server.sin_addr.s_addr = inet_addr(IP);
    server.sin_family = AF_INET;
    server.sin_port = htons((atoi(argv[2])));
    if (bind(sock, (struct sockaddr *)&server, sizeof(server)) < 0)
    {
        perror("bind(2)");
        close(sock);
        return 1;
    }
    printf("bind successfully\n");
    if (listen(sock, MAX_CLIENTS) < 0)
    {
        perror("listen(2)");
        close(sock);
        return 1;
    }

    if (strcmp(argv[4], "reno") == 0)
    {
        // set to be reno
        setsockopt(sock, IPPROTO_TCP, TCP_CONGESTION, "reno", strlen("reno"));
    }
    else if (strcmp(argv[4], "cubic") == 0)
    {
        // set to be cubic
        setsockopt(sock, IPPROTO_TCP, TCP_CONGESTION, "cubic", strlen("cubic"));
    }
    else
    {
        printf("Invalid TCP congestion control algorithm\n");
        return -1;
    }

    printf("Waiting for TCP connection...\n");
    printf("the chosen algo was: %s\n", argv[4]);
    int client_sock = accept(sock, (struct sockaddr *)&client, &client_len); // try to connect
    int filesize = 2097152;
    int bytes_received = 0;

    while (1)
    {
        bytes_received = 0;

        printf("enter to while\n");
        if (client_sock < 0)
        {
            perror("accept(2)");
            close(sock);
            return 1;
        }
        printf("connected\n");
        // Create a buffer to store the received message.
        char buffer[BUFFER_SIZE] = {0};
        start_t = clock();

        bytes_received < filesize ? puts("True" ) : puts("NotTrue" );

        // Receive a message from the client and store it in the buffer.
        while (bytes_received < filesize)
        {
            puts("here");
            int currBytes = recv(client_sock, buffer + bytes_received, BUFFER_SIZE - bytes_received, 0);

            bytes_received += currBytes;

            printf("now: %d - all time:%d\n", currBytes, bytes_received);

            // If the message receiving failed, print an error message and return 1.
            if (currBytes < 0)
            {
                perror("recv(2)");
                close(client_sock);
                close(sock);
                return 1;
            }

            else if (currBytes == 0)
            {
                fprintf(stdout, "Client %s:%d disconnected\n", inet_ntoa(client.sin_addr), (int)ntohs(client.sin_port));
                close(client_sock);
                close(sock);
                break;
            }

        }

        printf("size: %d", bytes_received);

        if (buffer[BUFFER_SIZE - 1] != '\0')
            buffer[BUFFER_SIZE - 1] = '\0';
        

        printf("recieved file\n");
        end_t = clock();
        total_t = (double)(end_t - start_t) / CLOCKS_PER_SEC;
        List_insertLast(dataList, total_t, (double)bytes_received / total_t);

        if (strcmp(buffer, "exit") == 0)
        {
            close(client_sock);
            printf(stdout, "Client %s:%d disconnected\n", inet_ntoa(client.sin_addr), ntohs(client.sin_port));
            close(sock);
            break;
        }
    }

    List_print(dataList);
    List_free(dataList);
    
    return 0;
}
