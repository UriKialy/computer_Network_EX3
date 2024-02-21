#include <stdio.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <unistd.h>
#include <string.h>
#define SERVER_IP "127.0.0.1"
#define SERVER_PORT 5060
#define BUFFER_SIZE 1024
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
int main()
{
    int sock = -1;
    struct sockaddr_in server;
    char buffer[BUFFER_SIZE] = {0};
    memset(&server, 0, sizeof(server));
    sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock == -1)
    {
        perror("socket(2)");
        return 1;
    }
    if (inet_pton(AF_INET, SERVER_IP, &server.sin_addr) <= 0)
    {
        perror("inet_pton(3)");
        close(sock);
        return 1;
    }
    server.sin_family = AF_INET;
    server.sin_port = htons(SERVER_PORT);
    if (connect(sock, (struct sockaddr *)&server, sizeof(server)) < 0)
    {
        perror("connect(2)");
        close(sock);
        return 1;
    }
    char *message = til_generate_random_data(2048);
    int bytes_sent = send(sock, message, strlen(message) + 1, 0);
    char *ans = printf("resend?");
    if (ans == "yes")
    {
        int bytes_sent = send(sock, message, strlen(message) + 1, 0);
    }
    else{
    char *exit="goodbye";
        int bytes_sent = send(sock, exit, strlen(exit) + 1, 0);
    }
    return 0;
}
