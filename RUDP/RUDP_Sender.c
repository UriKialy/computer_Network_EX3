#include <stdio.h>
#include <time.h>
#include <string.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <arpa/inet.h>
#include "stdlib.h"
#include <sys/socket.h>
#include <unistd.h>
#include "RUDP_API.h"

#define IP_ARG 2
#define PORT_ARG 4
#define ALGO_ARG 6
#define FILE_SIZE 2097152

/*
 * @brief A random data generator function based on srand() and rand().
 * @param size The size of the data to generate (up to 2^32 bytes).
 * @return A pointer to the buffer.
 */
char *util_generate_random_data(unsigned int size)
{
    char *buffer = NULL;
    // Argument check.
    if (size == 0)
        return NULL;
    buffer = (char *)calloc(size, sizeof(char));
    // Error checking.
    if (buffer == NULL)
        return NULL;
    // Randomize the seed of the random number generator.
    srand(time(NULL));
    for (unsigned int i = 0; i < size; i++)
        *(buffer + i) = ((unsigned int)rand() % 256);
    return buffer;
}

int main(int argc, char *argv[])
{

    // int port = 0;
    // char SERVER_IP[20] = {0};
    // if (argc != 5)
    // {
    //     printf("invalid command");
    //     return 1;
    // }
    // printf("Starting Sender...\n");
    // for (int i = 0; i <= argc; i++)
    // {
    //     if (strcmp(argv[i], "-p") == 0)
    //     {
    //         port = atoi(argv[i + 1]);
    //     }
    //     else if (strcmp(argv[i], "-ip") == 0)
    //     {
    //         strcpy(SERVER_IP, argv[i + 1]);
    //     }
    // }
    int bytesSent = 0;
    char ans[PORT_ARG] = "yes";
    RUDP_Socket *sock = rudp_socket(true, argv[PORT_ARG]);
    char *message = util_generate_random_data(FILE_SIZE);

    printf("Starting Sender.\n");

    printf("Connecting to Reciever...\n");
    int con = rudp_connect(sock, argv[IP_ARG], argv[PORT_ARG]);
    if (con < 0)
    {
        perror("Failed to connect");
        rudp_close(sock);
        return 1;
    }

    while (1)
    {
        bytesSent = 0;

        printf("Reciever connected, beginning to send file...\n");

        while (bytesSent < FILE_SIZE)
        {
            int seq = 0;
            int cur_sent = rudp_send(sock, message + bytesSent, FILE_SIZE - bytesSent, 0);
            if (cur_sent == -1)
            {

                perror("send(2)");
                rudp_close(sock);
                return 1;
            }
            else if (cur_sent == 0)
            {
                printf("peer has closed the TCP connection prior to send().\n");
                rudp_close(sock);
                return 1;
            }
            else if (cur_sent == -2)
            {
                continue;
            }
            else
            {
                bytesSent += cur_sent;
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

    }

    free(message);
    char *exit = "exit";
    bytesSent = rudp_send(sock, exit, strlen(exit) + 1, 0);
    rudp_disconnect(sock);
    rudp_close(sock);

    printf("Sender end.\n");

    return 0;
}

// Create socket
// Set up receiver address
// setting the SERVER_IP address
// Perform handshake
// Send data
// finish sending, and receive the last ack, close the rudp connection
// packet->flags = FLAG_FIN;
// to print stats