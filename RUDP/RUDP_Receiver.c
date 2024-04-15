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
#include "RUDP_API.h"

#define MAX_CLIENTS 1
#define PORT_ARG 2
#define ALGO_ARG 4
#define MUL 1000
#define DEV 1024
#define BUFFER_SIZE 2097152
#define IP "127.0.0.1"

int main(int argc, char *argv[])
{
    // if (argc != 3)
    // {
    //     printf("Illegal command!\n");
    //     return -1;
    // }
    // printf("Starting Receiver.\n");

    double total_t = 0;
    int bytes_received = 0;
    struct timeval start, end;
    char buffer[BUFFER_SIZE] = {0};
    List *dataList = List_alloc();

    RUDP_Socket *serverSock = rudp_socket(true, atoi(argv[PORT_ARG]));

    printf("Starting Receiver.\n");
    printf("Waiting for RUDP connection...\n");

    // try to connect
    int connect = rudp_accept(serverSock);
    printf("goot12\n");
    if (connect < 1)
    {
        printf("Couldn't accepted client, aborting...\n");
        rudp_close(serverSock);
        return -1;
    }

    while (1)
    {

        printf("Sender connected, beginning to receive file...\n");
        // Create a buffer to store the received message.

        gettimeofday(&start, NULL);

        // Receive a message from the client and store it in the buffer.
        while (bytes_received < BUFFER_SIZE)
        {
            int currBytes = rudp_recv(serverSock, buffer, BUFFER_SIZE);

            bytes_received += currBytes;

            // If the message receiving failed, print an error message and return 1.
            if (currBytes < 0)
            {
                perror("Receive was unsuccessful");
                rudp_close(serverSock);
                return 1;
            }

            else if (currBytes == 0)
            {
                fprintf(stdout, "Client disconnected\n");
                rudp_close(serverSock);
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
        printf("Waiting for Sender response...\n");
        rudp_recv(serverSock, buffer, sizeof(buffer));
        if (strcmp(buffer, "FIN") == 0)
        {

            printf("Sender sent exit message.\n");
            rudp_close(serverSock);
            break;
        }
    }

    List_print(dataList);
    List_free(dataList);

    printf("Receiver end.\n");

    return 0;
}