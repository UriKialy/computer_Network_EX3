#include "RUDP_API.h"

// Allocates a new structure for the RUDP socket (contains basic information about the socket itself). Also creates a UDP socket as a baseline for the RUDP.
// isServer means that this socket acts like a server. If set to server socket, it also binds the socket to a specific port.
RUDP_Socket *rudp_socket(bool isServer, unsigned short int listen_port)
{
    int sock = -1;
    RUDP_Socket *rudpSocket = malloc(sizeof(RUDP_Socket));

    if ((sock = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP)) == -1)
    {
        perror("socket(2)");
        return 1;
    }

    rudpSocket->isConnected = true; // sock is >0 so we are connected
    rudpSocket->isServer = isServer; // following the input (true/false)

    memset(&rudpSocket->dest_addr, 0, sizeof(rudpSocket->dest_addr));
    rudpSocket->dest_addr.sin_family = AF_INET;
    rudpSocket->dest_addr.sin_port = htons(listen_port);

    // if we create the socket from the server we will bind here
    if (isServer)
    {
        inet_pton(AF_INET, (const char *)SERVER_IP_ADDRESS, &(rudpSocket->dest_addr.sin_addr));
        
        if (bind(sock, (struct sockaddr *)&rudpSocket->dest_addr, sizeof(rudpSocket->dest_addr)) == -1)
        {
            perror("bind(2)");
            close(sock);
            return 1;
        }
    }

    return rudpSocket;
}

// Tries to connect to the other side via RUDP to given IP and port. Returns 0 on failure and 1 on success. Fails if called when the socket is connected/set to server.
int rudp_connect(RUDP_Socket *sockfd, const char *dest_ip, unsigned short int dest_port)
{
    if(sockfd->isConnected)
    {
        perror("Socket already connected\n");
        //TODO - CLOSE SOCKET
        return 1;
    }
    
    else if(sockfd->isServer){
        perror("Can't connect a server\n");
        //TODO - CLOSE SOCKET
        return 1;
    }
    
}