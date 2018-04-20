/* Author: Tyler Cook
 * Date: 25 April, 2018
 * UNT CSCE 3530
 * Description:
*/

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <time.h>

// #define SERVER "129.120.151.94"      // CSCE Server
#define SERVER "127.0.0.1"         // Local server
#define CLIENT "0.0.0.0"

typedef struct dhcp_pkt
{
    unsigned int siaddr;            // Server IP address
    unsigned int yiaddr;            // Client IP address
    unsigned int tran_ID;           // Transaction ID
    unsigned short int lifetime;    // Lease time
} dhcp_pkt;

void die(char *s);

void print_packet(struct dhcp_pkt *packet);

int main(int argc, char **argv)
{
    struct sockaddr_in si_other;
    struct sockaddr_in serv_ip_addr;
    struct sockaddr_in cli_ip_addr;
    int s, slen=sizeof(si_other);
    int portNumber;
    dhcp_pkt *readBuff, *sendBuff;

    // Verify we have our port number
    if (argc != 2)
    {
        printf("Error: Program usage: %s port_number\n", argv[0]);
        exit(1);
    }
    else
    {
        portNumber = atoi(argv[1]);
    }

    // Seed random number generator
    srand(time(NULL));

    // Set our socket
    if ( (s=socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP)) == -1)
    {
        die("socket");
    }

    // Set structure details
    memset((char *) &si_other, 0, sizeof(si_other));
    si_other.sin_family = AF_INET;
    si_other.sin_port = htons(portNumber);

    if (inet_aton(SERVER , &si_other.sin_addr) == 0)
    {
        fprintf(stderr, "inet_aton() failed\n");
        exit(1);
    }

    // Allocate memory for our messages
    readBuff = malloc(sizeof(dhcp_pkt));
    sendBuff = malloc(sizeof(dhcp_pkt));

    // Create our server and client IP Addresses
    inet_aton(SERVER, &sendBuff->siaddr);
    //inet_aton(CLIENT, &sendBuff->yiaddr);
    sendBuff->yiaddr = 0;

    //sendBuff->siaddr = &serv_ip_addr.sin_addr;
    //sendBuff->yiaddr = &cli_ip_addr.sin_addr;

    // Create DHCP discover packet
    sendBuff->tran_ID = rand();
    sendBuff->lifetime = 0;

    // Print this packet before send
    printf("Sending DHCP Discover packet to server\n");
    print_packet(sendBuff);

    // Send discover request to server
    if (sendto(s, sendBuff, sizeof(dhcp_pkt) , 0 , (struct sockaddr *) &si_other, slen) == -1)
    {
        die("sendto() error on request");
    }

    // Listen for offer
    if (recvfrom(s, readBuff, sizeof(dhcp_pkt), 0, (struct sockaddr *) &si_other, &slen) == -1)
    {
        die("recvfrom() error on offer");
    }

    // Print received packet
    printf("Received DHCP Offer packet from server\n");
    print_packet(readBuff);

    // Create DHCP request packet
    sendBuff->yiaddr = readBuff->yiaddr;
    sendBuff->tran_ID++;
    sendBuff->lifetime = readBuff->lifetime;

    // Print this packet before send
    printf("Sending DHCP Request packet to server\n");
    print_packet(sendBuff);

    // Send request
    if (sendto(s, sendBuff, sizeof(dhcp_pkt) , 0 , (struct sockaddr *) &si_other, slen) == -1)
    {
        die("sendto() error on request");
    }

    // Listen for ACK
    if (recvfrom(s, readBuff, sizeof(dhcp_pkt), 0, (struct sockaddr *) &si_other, &slen) == -1)
    {
        die("recvfrom() error on ACK");
    }

    // Print received packet
    printf("Received DHCP ACK packet from server\n");
    print_packet(readBuff);

    // Free memory
    free(readBuff);
    free(sendBuff);

    // Close the socket and exit
    close(s);
    return 0;
}

// Print error message and exit program
void die(char *s)
{
    perror(s);
    exit(1);
}

// Print the contents of a packet
void print_packet(struct dhcp_pkt *packet)
{
    struct in_addr ipstore;

    ipstore.s_addr = packet->siaddr;
    printf("Server IP: %s\n", inet_ntoa(ipstore));

    ipstore.s_addr = packet->yiaddr;
    printf("Client IP: %s\n", inet_ntoa(ipstore));

    printf("Transaction ID: %u\n", packet->tran_ID);
    printf("Lifetime: %u\n", packet->lifetime);
}
