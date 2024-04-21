#include <stdio.h>
#include <time.h>
#include <string.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <arpa/inet.h>
#include "stdlib.h"
#include <sys/socket.h>
#include <unistd.h>
#include <regex.h>
#include "RUDP_API.h"

#define IP_ARG 2
#define PORT_ARG 4
#define FILE_SIZE 2097152

const char *ipv4_regex = "^(([0-9]|[1-9][0-9]|1[0-9][0-9]|2[0-4][0-9]|25[0-5])\\.){3}([0-9]|[1-9][0-9]|1[0-9][0-9]|2[0-4][0-9]|25[0-5])$";
const char *portaddr_regex = "^([0-9]{1,4}|[1-5][0-9]{4}|6[0-4][0-9]{3}|65[0-4][0-9]{2}|655[0-2][0-9]|6553[0-5])$";

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
    if (strcmp(argv[PORT_ARG - 1], "-p") || strcmp(argv[IP_ARG - 1], "-ip"))
    {
        printf("Invalid Arguments!\n");
        exit(EXIT_FAILURE);
    }

    regex_t ip_regex, port_regex;
    int ip_result, port_result;

    // Compile IP address regex
    ip_result = regcomp(&ip_regex, ipv4_regex, REG_EXTENDED);
    if (ip_result != 0)
    {
        fprintf(stderr, "Error compiling IP regex: %s\n", strerror(ip_result));
        return 1;
    }

    // Compile port number regex
    port_result = regcomp(&port_regex, portaddr_regex, REG_EXTENDED);
    if (port_result != 0)
    {
        fprintf(stderr, "Error compiling port regex: %s\n", strerror(port_result));
        regfree(&ip_regex); // Free previously compiled regex
        return 1;
    }

    // Match IP address
    ip_result = regexec(&ip_regex, argv[IP_ARG], 0, NULL, 0);
    if (ip_result != 0)
    {
        printf("%s is not a valid IP address\n", argv[IP_ARG]);
        regfree(&ip_regex);
        regfree(&port_regex);
        return 1;
    }

    // Match port number
    port_result = regexec(&port_regex, argv[PORT_ARG], 0, NULL, 0);
    if (port_result != 0)
    {
        printf("%s is not a valid port number\n", argv[PORT_ARG]);
        exit(EXIT_FAILURE);
    }
    else
    {
        printf("Valid IP address: %s\n", argv[IP_ARG]);
        printf("Valid port number: %s\n", argv[PORT_ARG]);
    }

    // Free compiled regexes
    regfree(&ip_regex);
    regfree(&port_regex);

    int seq = 0;
    int bytesSent = 0;
    char ans[PORT_ARG] = "yes";
    RUDP_Socket *sock = rudp_socket(false, atoi(argv[PORT_ARG])); // create a socket
    char *message = util_generate_random_data(FILE_SIZE);   // generate random data

    printf("Starting Sender.\n");

    printf("Connecting to Reciever...\n");
    int connect = rudp_connect(sock, argv[IP_ARG], (short)atoi(argv[PORT_ARG])); // connect to reciever
    if (connect < 0)
    {
        printf("Failed to connect.\n");
        rudp_close(sock);
        return 1;
    }

    printf("Reciever connected!\n");

    while (1)
    {
        seq = 0;
        bytesSent = 0;

        printf("Beginning to send file...\n");

        while (bytesSent < FILE_SIZE) // as long as the file is not fully sent
        {

            int cur_sent = rudp_send(sock, message + bytesSent, FILE_SIZE - bytesSent, seq); // send the each chunk of the file
            if (cur_sent == -1)
            {
                printf("send failed.\n");
                rudp_close(sock);
                return 1;
            }
            else if (cur_sent == 0)
            {
                printf("peer has closed the RUDP connection prior to send().\n");
                rudp_close(sock);
                return 1;
            }
            else if (cur_sent == -2) // if the file is not fully sent
            {
                continue;
            }
            else
            {
                printf("diff - %d\n", (cur_sent - seq));
                bytesSent += (cur_sent - seq); // increment the bytes sent
            }

            seq = cur_sent;
        }
        printf("File was successfully sent.\n");
        printf("Do you want to resend the file? [yes/no]: ");
        scanf("%s", ans);
        if (ans[0] != 'y')
        {
            break;
        }

        rudp_send(sock, "CON", 3, 0);
    }
    // free the memory and close the socket
    free(message);
    rudp_disconnect(sock);
    rudp_close(sock);

    printf("Sender end.\n");

    return 0;
}
