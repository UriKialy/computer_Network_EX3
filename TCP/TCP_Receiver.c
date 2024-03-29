#include <stdio.h>
#include <stdlib.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <unistd.h>
#include <string.h>
#include <time.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <sys/time.h>
#include "List.c"

#define MAX_CLIENTS 1
#define PORT_ARG 2
#define ALGO_ARG 4
#define MUL 1000
#define DEV 1024
#define BUFFER_SIZE 2097152
#define IP "127.0.0.1"

int main(int argc, char *argv[])
{
    struct timeval start, end;
    int bytes_received = 0;
    List *dataList = List_alloc();
    struct sockaddr_in server;
    struct sockaddr_in client;
    socklen_t client_len = sizeof(client);
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
    server.sin_port = htons((atoi(argv[PORT_ARG])));
    if (bind(sock, (struct sockaddr *)&server, sizeof(server)) < 0)
    {
        perror("bind(2)");
        close(sock);
        return 1;
    }

    if (listen(sock, MAX_CLIENTS) < 0)
    {
        perror("listen(2)");
        close(sock);
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

    printf("Waiting for TCP connection...\n");
    int client_sock = accept(sock, (struct sockaddr *)&client, &client_len); // try to connect

    while (1)
    {
        bytes_received = 0;

        if (client_sock < 0)
        {
            perror("accept(2)");
            close(sock);
            return 1;
        }
        printf("Sender connected, beginning to receive file...\n");
        // Create a buffer to store the received message.
        char buffer[BUFFER_SIZE] = {0};
        gettimeofday(&start, NULL);

        // Receive a message from the client and store it in the buffer.
        while (bytes_received < BUFFER_SIZE)
        {
            int currBytes = recv(client_sock, buffer + bytes_received, BUFFER_SIZE - bytes_received, 0);

            bytes_received += currBytes;

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

        if (buffer[BUFFER_SIZE - 1] != '\0')
            buffer[BUFFER_SIZE - 1] = '\0';

        printf("File transfer completed.\n");

        gettimeofday(&end, NULL);
        total_t = ((end.tv_sec - start.tv_sec) * 1000 + ((double)(end.tv_usec - start.tv_usec) / 1000));
        double bandwith = ((double)(BUFFER_SIZE / 1024) / 1024) / (total_t / 1000);
        List_insertLast(dataList, total_t, bandwith);

        send(client_sock, "\0", 2, 0);

        printf("Waiting for Sender response...\n");

        char buff[32] = { 0 };

        recv(client_sock, buff, sizeof(buff), 0);

        if (strcmp(buff, "exit") == 0)
        {
            printf("Sender sent exit message.\n");
            close(client_sock);
            close(sock);
            break;
        }

        send(client_sock, "\0", 2, 0);
    }

    List_print(dataList);
    List_free(dataList);

    printf("Receiver end.\n");

    return 0;
}
