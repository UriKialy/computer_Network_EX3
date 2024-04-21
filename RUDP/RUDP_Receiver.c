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
#include <regex.h>

#include "List.h"
#include "RUDP_API.h"

const char *port_regex = "^([0-9]{1,4}|[1-5][0-9]{4}|6[0-4][0-9]{3}|65[0-4][0-9]{2}|655[0-2][0-9]|6553[0-5])$";

#define MAX_CLIENTS 1
#define PORT_ARG 2
#define MUL 1000
#define DEV 1024
#define DATA_SIZE 2097152

int main(int argc, char *argv[])
{
    if (strcmp(argv[PORT_ARG - 1], "-p"))
    {
        printf("Invalid Arguments!\n");
        exit(EXIT_FAILURE);
    }

    regex_t regex;
    int result;
    char port_number[] = "443"; // Replace with the port number to validate

    // Compile the regular expression
    result = regcomp(&regex, port_regex, REG_EXTENDED);
    if (result != 0)
    {
        fprintf(stderr, "Error compiling regex: %s\n", strerror(result));
        return 1;
    }

    // Match the port number against the regex
    result = regexec(&regex, argv[PORT_ARG], 0, NULL, 0);

    // Check the match result
    if (result == 0)
    {
        printf("%s is a valid port number\n", argv[PORT_ARG]);
    }
    else if (result == REG_NOMATCH)
    {
        printf("%s is not a valid port number\n", argv[PORT_ARG]);
        exit(EXIT_FAILURE);
    }
    else
    {
        fprintf(stderr, "Regex matching error: %s\n", strerror(result));
        exit(EXIT_FAILURE);
    }

    // Free the compiled regex
    regfree(&regex);

    double total_t = 0;
    int bytes_received = 0;
    struct timeval start, end;
    char buffer[DATA_SIZE] = {0};                                             // Buffer to store the message from the client.
    List *dataList = List_alloc();                                            // List to store the data.
    RUDP_Socket *serverSock = rudp_socket(true, (short)atoi(argv[PORT_ARG])); // Create a new RUDP socket.
    printf("Starting Receiver.\n");
    printf("Waiting for RUDP connection...\n");

    // try to connect
    int connect = rudp_accept(serverSock);
    if (connect < 1)
    {
        printf("Couldn't accepted client, aborting...\n");
        rudp_close(serverSock);
        return -1;
    }

    printf("Sender connected, beginning to receive file...\n");

    while (1)
    {

        bytes_received = 0;

        gettimeofday(&start, NULL);

        while (bytes_received < DATA_SIZE)
        {
            // Receive a message from the client and store it in the buffer.
            int currBytes = rudp_recv(serverSock, buffer + bytes_received, DATA_SIZE - bytes_received);
            bytes_received += currBytes; // Increment the number of bytes received.

            // If the message receiving failed, print an error message and return 1.
            if (currBytes < 0)
            {
                printf("Receive was unsuccessful\n");
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

        if (buffer[DATA_SIZE - 1] != '\0') // If the last character of the buffer is not a null character, set it to a null character.
            buffer[DATA_SIZE - 1] = '\0';

        printf("File transfer completed.\n");

        gettimeofday(&end, NULL);
        total_t = ((end.tv_sec - start.tv_sec) * MUL + ((double)(end.tv_usec - start.tv_usec) / MUL)); // Calculate the total time taken to receive the file.
        double bandwith = ((double)(DATA_SIZE / DEV) / DEV) / (total_t / MUL);                         // Calculate the bandwith.
        List_insertLast(dataList, total_t, bandwith);                                                  // Insert the total time and bandwith into the list.

        printf("Waiting for Sender response...\n");

        int fin = rudp_recv(serverSock, buffer, sizeof(buffer)); // Receive a message from the client and check if its an exit message.
        if (fin == -2)
        {

            printf("Sender sent exit message.\n");
            rudp_close(serverSock);
            break;
        }
        else
        {
            continue;
        }
    }
    // Print the data list and free the list.
    List_print(dataList);
    List_free(dataList);

    printf("Receiver end.\n");

    return 0;
}