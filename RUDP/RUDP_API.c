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
    timeout.tv_sec = 10;
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
        rclose(sockfd->socket_fd);
        return 0;
    }

    sockfd->dest_addr.sin_port = htons(dest_port);
    if (inet_pton(AF_INET, dest_ip, &sockfd->dest_addr.sin_addr) <= 0)
    {
        printf("inet_pton() failed");
        return -1;
    }

    pack->syn = 1;

    if (setsockopt(sockfd->socket_fd, SOL_SOCKET, SO_RCVTIMEO, &timeout, sizeof(timeout)) < 0)
    {
        perror("setsockopt(2)");
        return 0;
    }
    for (int i = 0; i < 5; i++)
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

        if (!pack->syn || !pack->ack)
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
    timeout.tv_sec = 10;
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
    int ack = sendto(sockfd->socket_fd, (void *)pack, BUFFER_SIZE, 0, (struct sockaddr_in *)&sockfd->dest_addr, sizeof(sockfd->dest_addr));
}

// Receives data from the other side and put it into the buffer. Returns the number of received bytes on success, 0 if got FIN packet (disconnect), and -1 on error.
// Fails if called when the socket is disconnected.
int rudp_recv(RUDP_Socket *sockfd, void *buffer, unsigned int buffer_size) {}

// Sends data stores in buffer to the other side. Returns the number of sent bytes on success, 0 if got FIN packet (disconnect), and -1 on error. Fails if called when the socket is disconnected.
int rudp_send(RUDP_Socket *sockfd, void *buffer, unsigned int buffer_size) {}

// Disconnects from an actively connected socket. Returns 1 on success, 0 when the socket is already disconnected (failure).
int rudp_disconnect(RUDP_Socket *sockfd) {}

// This function releases all the memory allocation and resources of the socket.
int rudp_close(RUDP_Socket *sockfd) {}

///-------HELPER FUNCTIONS--------///

RUDP_Packet *create_Packet()
{
    RUDP_Packet *packet;
    packet->length = 0;
    packet->checksum = 0;
    packet->ack = 0;
    packet->fin = 0;
    packet->syn = 0;
    packet->seq = 0;
    packet->mes = 0;
    return packet;
}
