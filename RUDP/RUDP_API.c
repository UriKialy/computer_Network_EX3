#include "RUDP_API.h"

///-------HELPER FUNCTIONS--------///

RUDP_Packet *create_Packet(void)
{
    RUDP_Packet *packet = calloc(1, sizeof(RUDP_Packet));

    if (packet == NULL)
    {
        perror("no memory");
        exit(EXIT_FAILURE);
    }

    packet->header = (RUDP_Header *)calloc(1, sizeof(RUDP_Header));
    if (packet->header == NULL)
    {
        perror("no memory");
        free(packet);
        exit(EXIT_FAILURE);
    }

    // memset(&packet, 0, sizeof(packet));
    //  (packet->header)=(RUDP_Header*)malloc(sizeof(RUDP_Header));
    //  if(packet->header==NULL){
    //      perror("no memory");
    //      free(packet);
    //      return NULL;
    //  }
    // memset(packet->header,0,sizeof(packet->header));

    // packet->header->length = 0;
    // packet->header->checksum = 0;
    // packet->header->ack = 0;
    // packet->header->fin = 0;
    // packet->header->syn = 0;
    // packet->header->seq = 0;
    // *(packet->mes) = 0;

    return packet;
}

void set_Packet(RUDP_Packet *packet, char ack, char fin, char syn, short seq, char *mes)
{
    // memset(packet->header->ack, ack, sizeof(char));
    // memset(packet->header->fin, fin, sizeof(char));
    // memset(packet->header->syn, syn, sizeof(char));
    // memset(packet->header->seq, seq, sizeof(short));
    packet->header->fin = fin;
    packet->header->syn = syn;
    packet->header->seq = seq;
    packet->header->ack = ack;
    if (strchr(mes, '\0') == NULL)
    { // check if the messenge is ending with \0
        perror("messenge too long wtf");
        exit(EXIT_FAILURE);
    }
    strcpy(packet->mes, mes);
    packet->header->length = strlen(mes);
    packet->header->checksum = calculate_checksum((void *)packet, packet->header->length), sizeof(short);
    // memset(packet->header->checksum, calculate_checksum((void *)packet, packet->header->length), sizeof(short));
}

int send_fin(RUDP_Socket *sockfd)
{
    RUDP_Packet *pack = create_Packet();
    set_Packet(pack, 0, 1, 0, 0, "FIN");
    int bytes_send = sendto(sockfd->socket_fd, (void *)pack, BUFFER_SIZE, 0, (const struct sockaddr *)&sockfd->dest_addr, sizeof(sockfd->dest_addr));
    if (bytes_send == 0)
    {
        printf("connection closed.\n");
        free(pack);
        return -1;
    }
    else if (bytes_send < 0)
    {
        perror("send(2)");
        free(pack);
        return 0;
    }
    free(pack);
    return 1;
}

int send_ack(RUDP_Socket *sockfd, int seq)
{
    RUDP_Packet *pack = create_Packet();
    set_Packet(pack, 1, 0, 0, seq + 1, "ACK");
    int bytes_send = sendto(sockfd->socket_fd, (void *)pack, BUFFER_SIZE, 0, (struct sockaddr *)&sockfd->dest_addr, sizeof(sockfd->dest_addr));
    if (bytes_send == 0)
    {
        printf("connection closed.\n");
        free(pack);
        return -1;
    }
    else if (bytes_send < 0)
    {
        perror("send(2)");
        free(pack);
        return 0;
    }
    free(pack);
    return 1;
}

void free_packet(RUDP_Packet *p)
{
    if (p == NULL)
    {
        return;
    }
    if (p->header != NULL)
        free(p->header);
    free(p);
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

    rudpSocket->isConnected = false; // sock is > 0 so we are connected
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
    timeout.tv_sec = 0;
    timeout.tv_usec = TIMEOUT_MICROSECS;

    if (sockfd->isConnected)
    {
        perror("Socket already connected\n");
        close(sockfd->socket_fd);
        free_packet(pack);
        return 0;
    }

    else if (sockfd->isServer)
    {
        perror("Can't connect a server\n");
        close(sockfd->socket_fd);
        free_packet(pack);
        return 0;
    }

    sockfd->dest_addr.sin_port = htons(dest_port);
    if (inet_pton(AF_INET, dest_ip, &(sockfd->dest_addr.sin_addr)) < 0)
    {
        perror("error, inet_pton(3)");
        free_packet(pack);
        return -1;
    }
    else if (inet_pton(AF_INET, dest_ip, &(sockfd->dest_addr.sin_addr)) == 0)
    {
        perror("error, inet_pton(3)");
        free_packet(pack);
        return -1;
    }

    set_Packet(pack, 0, 0, 1, 0, "SYN");
    print_syn_packet(pack);

    for (int i = 0; i < TIMES_TO_SEND; i++)
    {
        if (setsockopt(sockfd->socket_fd, SOL_SOCKET, SO_RCVTIMEO, &timeout, sizeof(timeout)) < 0)
        {
            perror("setsockopt(2)");
            free_packet(pack);
            return 0;
        }
        int ack = sendto(sockfd->socket_fd, pack, sizeof(RUDP_Packet), 0, (struct sockaddr *)&sockfd->dest_addr, sizeof(sockfd->dest_addr));
        if (ack == 0)
        {
            printf("send failed.\n");
            free_packet(pack);
            return 0;
        }
        else if (ack == -1)
        {
            perror("send(2)");
            free_packet(pack);
            return 0;
        }
        int syn_Ack = recvfrom(sockfd->socket_fd, pack, sizeof(RUDP_Packet), 0, NULL, 0);
        if (syn_Ack == 0)
        {
            printf("Receive failed.\n");
            free_packet(pack);
            return 0;
        }
        else if (syn_Ack == -1)
        {
            free_packet(pack);
            return -1;
        }

        if (!pack->header->syn || !pack->header->ack)
        {
            printf("Didn't Received SYN-ACK.\n");
            free_packet(pack);
            return 0;
        }
    }

    sockfd->isConnected = true;
    free_packet(pack);
    return 1;
}

// Accepts incoming connection request and completes the handshake, returns 0 on failure and 1 on success. Fails if called when the socket is connected/set to client.
// No changes needed to struct definitions (already provided)

// Implementations for create_Packet and set_Packet (assuming they are not shown)

int rudp_accept(RUDP_Socket *serverSock)
{
    struct sockaddr_storage their_addr;
    socklen_t addr_size = sizeof(their_addr);
    fd_set readfds;
    struct timeval timeout;
    RUDP_Packet  *recvPacket= create_Packet();
    // Set timeout for receiving connection request
    timeout.tv_sec = 0;
    timeout.tv_usec = TIMEOUT_MICROSECS;

    while (1)
    {
        FD_ZERO(&readfds);
        FD_SET(serverSock->socket_fd, &readfds);

        // Wait for data or timeout
        int select_ret = select(serverSock->socket_fd + 1, &readfds, NULL, NULL, &timeout);
        if (select_ret < 0)
        {
            perror("select");
            return NULL;
        }
        else if (select_ret == 0)
        { // Timeout occurred
            printf("Timeout waiting for connection request\n");
            continue;
        }

        // Data available on the socket (serverSock->sockfd)
        int bytes_recvd = recvfrom(serverSock->socket_fd, recvPacket, BUFFER_SIZE, 0, (struct sockaddr *)&their_addr, &addr_size);

        // Check for errors during receive
        if (bytes_recvd < 0)
        {
            if (errno == EWOULDBLOCK)
            { // Timeout occurred
                printf("Timeout waiting for connection request\n");
                continue;
            }
            else
            {
                perror("recvfrom");
                return NULL;
            }
        }

        // Process received data (assuming valid RUDP packet format)
        if (recvPacket->header->syn && !(recvPacket->header->ack))
        {
            // Valid connection request (SYN packet)
            //serverSock->dest_addr = (struct sockaddr_in *)&their_addr;
            serverSock->dest_addr.sin_family = ((struct sockaddr_in *)&their_addr)->sin_family;
            serverSock->dest_addr.sin_port = ((struct sockaddr_in *)&their_addr)->sin_port;
            // ... copy other relevant fields if needed

            break;
        }
        else
        {
            printf("Unexpected packet received during connection setup, ignoring\n");
        }
    }

    // Send a SYN-ACK response to the sender
    set_Packet(recvPacket, 1, 0, 1, 0, "SYN-ACK");
    if (sendto(serverSock->socket_fd, recvPacket, BUFFER_SIZE, 0, (struct sockaddr *)&serverSock->dest_addr, addr_size) < 0)
    {
        perror("sendto");
        return NULL;
    }

    printf("Connection established with client.\n");
    return 1; // Return the received SYN packet (optional)
}



// Receives data from the other side and put it into the buffer. Returns the number of received bytes on success, 0 if got FIN packet (disconnect), and -1 on error.
// Fails if called when the socket is disconnected.
int rudp_recv(RUDP_Socket *sockfd, void *buffer, unsigned int buffer_size)
{
    socklen_t dest_addr_len = sizeof(struct sockaddr);
    RUDP_Packet *receivePacket = create_Packet();
    struct timeval timeout;
    timeout.tv_sec = 0;
    timeout.tv_usec = TIMEOUT_MICROSECS;

    if (!sockfd->isConnected)
    {
        free_packet(receivePacket);
        return -1;
    }
    int bytes_rec = recvfrom(sockfd->socket_fd, receivePacket, sizeof(RUDP_Packet), 0, (struct sockaddr_in *)&sockfd->dest_addr, dest_addr_len);
    if (bytes_rec < 0)
    {
        free_packet(receivePacket);
        return -1;
    }
    else if (bytes_rec == 0)
    {
        free_packet(receivePacket);
        fprintf(stdout, "%s:%d disconnected\n", inet_ntoa(sockfd->dest_addr.sin_addr), (int)ntohs(sockfd->dest_addr.sin_port));
        close(sockfd->socket_fd);
        exit(1);
    }

    if (calculate_checksum(receivePacket, receivePacket->header->length) != receivePacket->header->checksum)
    {
        perror("Checksum wasn't the same as the calc, error");
        free_packet(receivePacket);
        return -1;
    }
    if (receivePacket->header->fin)
    {
        printf("Recieved FIN, disconnecting\n");
        free_packet(receivePacket);
        return 0;
    }

    strncpy(buffer, receivePacket->mes, buffer_size);

    send_ack(sockfd, receivePacket->header->seq);
    free_packet(receivePacket);

    return bytes_rec;
}

// Sends data stores in buffer to the other side. Returns the number of sent bytes on success, 0 if got FIN packet (disconnect), and -1 on error. Fails if called when the socket is disconnected.
int rudp_send(RUDP_Socket *sockfd, void *buffer, unsigned int buffer_size, unsigned short seq)
{
    int bytes_sent = 0;
    int timeout_oc = 0;
    RUDP_Packet *pack = create_Packet();
    struct timeval timeout;
    timeout.tv_sec = 0;
    timeout.tv_usec = 500;

    if (sockfd->isConnected)
    {
        perror("Socket already connected\n");
        close(sockfd->socket_fd);
        free_packet(sockfd);
        return 0;
    }
    else if (!sockfd->isServer)
    {
        perror("Can't accept a client\n");
        close(sockfd->socket_fd);
        free_packet(sockfd);

        return 0;
    }

    set_Packet(pack, 0, 0, 1, seq, 0);

    for (int i = 0; i < TIMES_TO_SEND; i++)
    {
        // void *=strchr(buffer,'0');
        bytes_sent = sendto(sockfd->socket_fd, pack, BUFFER_SIZE, 0, (struct sockaddr_in *)&sockfd->dest_addr, sizeof(sockfd->dest_addr));

        if (bytes_sent == 0)
        {
            printf("Receive failed.\n");
            free_packet(pack);
            return 0;
        }
        else if (bytes_sent == -1)
        {
            if (errno == EWOULDBLOCK || errno == EAGAIN)
            {
                timeout_oc = 1;
            }
            else
            {
                perror("recvfrom(2)");
                free_packet(pack);
                return 0;
            }
        }
        if (timeout_oc)
        {
            perror("Timout occured - Aborting Connection\n");
            free_packet(pack);
            return -2;
        }

        int ack = recvfrom(sockfd->socket_fd, pack, BUFFER_SIZE, 0, (struct sockaddr_in *)&sockfd->dest_addr, sizeof(sockfd->dest_addr));
        if (ack < 0)
        {
            if (ack == 0)
            {
                printf("Receive failed.\n");
                free_packet(pack);
                return 0;
            }
            else if (ack == -1)
            {
                free_packet(pack);
                return -1;
            }
        }
    }

    free_packet(pack);
    return bytes_sent;
}

// Disconnects from an actively connected socket. Returns 1 on success, 0 when the socket is already disconnected (failure).
int rudp_disconnect(RUDP_Socket *sockfd)
{
    RUDP_Packet *pack = create_Packet();

    if (!sockfd->isConnected)
    {
        perror("Socket already disconnected");
        free_packet(pack);
        return 0;
    }
    int sent = send_fin(sockfd);
    int receive_bytes = recvfrom(sockfd->socket_fd, pack, BUFFER_SIZE, 0, (struct sockaddr_in *)&sockfd->dest_addr, sizeof(sockfd->dest_addr));

    if (sent && pack->header->ack)
    {
        sockfd->isConnected = false;
        free_packet(pack);
        return 1;
    }
}

// This function releases all the memory allocation and resources of the socket.
int rudp_close(RUDP_Socket *sockfd)
{
    close(sockfd->socket_fd);
    free(sockfd);
}

void print_syn_packet(RUDP_Packet *packet)
{
    printf("SYN: %d\n", packet->header->syn);
}

//    int timeout_oc = 0;
// if (bytes_rec < 0)
// {
//     if (errno == EWOULDBLOCK || errno == EAGAIN)
//     {
//         timeout_oc = 1;
//     }
//     else
//     {
//         perror("recvfrom(2)");
//         close(sockfd->socket_fd);
//         return -1;
//     }
// }
// if (timeout_oc)
// {
//     perror("Timout occured - Aborting Connection\n");
//     return -1;
// }