#include "RUDP_API.h"

// Allocates a new structure for the RUDP socket (contains basic information about the socket itself). Also creates a UDP socket as a baseline for the RUDP.
// isServer means that this socket acts like a server. If set to server socket, it also binds the socket to a specific port.
RUDP_Socket *rudp_socket(bool isServer, unsigned short int listen_port)
{
    RUDP_Socket *rudpSocket = malloc(sizeof(RUDP_Socket));
    // rudpSocket->socket_fd = -1;

    if ((rudpSocket->socket_fd = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP)) == -1)
    {
        perror("socket(2)");
        exit(1);
    }

    rudpSocket->isConnected = true;  // sock is > 0 so we are connected
    rudpSocket->isServer = isServer; // following the input (true/false)

    memset(&rudpSocket->dest_addr, 0, sizeof(rudpSocket->dest_addr));
    rudpSocket->dest_addr.sin_family = AF_INET;

    // if we create the socket from the server we will bind here
    if (isServer)
    {
        rudpSocket->dest_addr.sin_port = htons(listen_port);
        // inet_pton(AF_INET, (const char *)SERVER_IP_ADDRESS, &(rudpSocket->dest_addr.sin_addr));

        if (bind(rudpSocket->socket_fd, (struct sockaddr *)&rudpSocket->dest_addr, sizeof(rudpSocket->dest_addr)) == -1)
        {
            perror("bind(2)");
            close(rudpSocket->socket_fd);
            exit(1);
        }
    }

    return rudpSocket;
}

// Tries to connect to the other side via RUDP to given IP and port. Returns 0 on failure and 1 on success. Fails if called when the socket is connected/set to server.
int rudp_connect(RUDP_Socket *sockfd, const char *dest_ip, unsigned short int dest_port)
{
    RUDP_Packet *pack = create_Packet();
    struct timeval timeout;
    timeout.tv_sec = 1;
    timeout.tv_usec = 0;

    if (sockfd->isConnected)
    {
        perror("Socket already connected\n");
        close(sockfd->socket_fd);
        return 0;
    }

    else if (sockfd->isServer)
    {
        perror("Can't connect a server\n");
        close(sockfd->socket_fd);
        return 0;
    }

    sockfd->dest_addr.sin_port = htons(dest_port);
    if (inet_pton(AF_INET, dest_ip, &sockfd->dest_addr.sin_addr) <= 0)
    {
        printf("inet_pton() failed");
        return -1;
    }
    set_Packet(pack, 1, 0, 0, 0, 0);

    if (setsockopt(sockfd->socket_fd, SOL_SOCKET, SO_RCVTIMEO, &timeout, sizeof(timeout)) < 0)
    {
        perror("setsockopt(2)");
        return 0;
    }

    for (int i = 0; i < TIMES_TO_SEND; i++)
    {
        int ack = sendto(sockfd->socket_fd, (void *)pack, BUFFER_SIZE, 0, (struct sockaddr_in *)&sockfd->dest_addr, sizeof(sockfd->dest_addr));
        if (ack == 0)
        {
            printf("send failed.\n");
            return 0;
        }
        else if (ack == -1)
        {
            perror("send(2)");
            return 0;
        }
        int syn_Ack = recvfrom(sockfd->socket_fd, (void *)pack, BUFFER_SIZE, 0, (struct sockaddr_in *)&sockfd->dest_addr, sizeof(sockfd->dest_addr));

        if (!pack->header->syn || !pack->header->ack)
        {
            printf("Didn't Received SYN-ACK.\n");
            return 0;
        }
    }
}

// Accepts incoming connection request and completes the handshake, returns 0 on failure and 1 on success. Fails if called when the socket is connected/set to client.
int rudp_accept(RUDP_Socket *sockfd)
{
    RUDP_Packet *pack = create_Packet();
    struct timeval timeout;
    timeout.tv_sec = 1;
    timeout.tv_usec = 0;

    if (sockfd->isConnected)
    {
        perror("Socket already connected\n");
        close(sockfd->socket_fd);
        return 0;
    }
    else if (!sockfd->isServer)
    {
        perror("Can't accept a client\n");
        close(sockfd->socket_fd);
        return 0;
    }

    int syn_Ack = recvfrom(sockfd->socket_fd, (void *)pack, BUFFER_SIZE, 0, (struct sockaddr_in *)&sockfd->dest_addr, sizeof(sockfd->dest_addr));
    if (syn_Ack == 0)
    {
        printf("Receive failed.\n");
        return 0;
    }
    else if (syn_Ack == -1)
    {
        perror("recv(2)");
        return 0;
    }
    if (!pack->header->ack)
    {
        printf("Didn't Received SYN-ACK.\n");
        return 0;
    }

    set_Packet(pack, 1, 0, 1, 0, 0);
    int ack = sendto(sockfd->socket_fd, (void *)pack, BUFFER_SIZE, 0, (struct sockaddr_in *)&sockfd->dest_addr, sizeof(sockfd->dest_addr));
}

// Receives data from the other side and put it into the buffer. Returns the number of received bytes on success, 0 if got FIN packet (disconnect), and -1 on error.
// Fails if called when the socket is disconnected.
int rudp_recv(RUDP_Socket *sockfd, void *buffer, unsigned int buffer_size)
{
}

// Sends data stores in buffer to the other side. Returns the number of sent bytes on success, 0 if got FIN packet (disconnect), and -1 on error. Fails if called when the socket is disconnected.
int rudp_send(RUDP_Socket *sockfd, void *buffer, unsigned int buffer_size) {}

// Disconnects from an actively connected socket. Returns 1 on success, 0 when the socket is already disconnected (failure).
int rudp_disconnect(RUDP_Socket *sockfd) {}

// This function releases all the memory allocation and resources of the socket.
int rudp_close(RUDP_Socket *sockfd) {}

///-------HELPER FUNCTIONS--------///

RUDP_Packet *create_Packet(void)
{
    RUDP_Packet *packet;

    packet->header->length = 0;
    packet->header->checksum = 0;
    packet->header->ack = 0;
    packet->header->fin = 0;
    packet->header->syn = 0;
    packet->header->seq = 0;
    *(packet->mes) = 0;

    return packet;
}
void set_Packet(RUDP_Packet *packet, char ack, char fin, char syn, short seq, char mes[BUFFER_SIZE])
{
    packet->header->ack = ack;
    packet->header->fin = fin;
    packet->header->syn = syn;
    packet->header->seq = seq;
    strcpy(*packet->mes, mes);
    packet->header->length = strlen(mes) + sizeof(RUDP_Header);
    packet->header->checksum = calculate_checksum((void *)packet, strlen(mes) + sizeof(RUDP_Header));
}

unsigned short int calculate_checksum(void *data, unsigned int bytes)
{
    unsigned short int *data_pointer = (unsigned short int *)data;
    unsigned int total_sum = 0;
    // Main summing loop
    while (bytes > 1)
    {
        total_sum += *data_pointer++;
        bytes -= 2;
    }
    // Add left-over byte, if any
    if (bytes > 0)
        total_sum += *((unsigned char *)data_pointer);
    // Fold 32-bit sum to 16 bits
    while (total_sum >> 16)
        total_sum = (total_sum & 0xFFFF) + (total_sum >> 16);
    return (~((unsigned short int)total_sum));
}