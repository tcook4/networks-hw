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

#define SERVER "129.120.151.94"
#define BUFLEN 512  //Max length of buffer

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
    int s, i, slen=sizeof(si_other);
    char buf[BUFLEN];
    char message[BUFLEN];
    char *quit = "quit";
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

    // Create DHCP discover packet
    sendBuff->siaddr = 12912015194;
    sendBuff->yiaddr = 0000;
    sendBuff->tran_ID = rand();
    sendBuff->lifetime = 0;

    // Print this packet before send
    printf("Sending DHCP Discover packet to server\n");
    print_packet(sendBuff);

    // Send discover request to server
    if (sendto(s, sendBuff, sizeof(dhcp_pkt) , 0 , (struct sockaddr *) &si_other, slen) == -1)
    {
        die("sendto()");
    }

    // Listen for offer
    if (recvfrom(s, readBuff, sizeof(dhcp_pkt), 0, (struct sockaddr *) &si_other, &slen) == -1)
    {
        die("recvfrom()");
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
        die("sendto()");
    }

    // Listen for ACK
    if (recvfrom(s, readBuff, sizeof(dhcp_pkt), 0, (struct sockaddr *) &si_other, &slen) == -1)
    {
        die("recvfrom()");
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
    printf("%u : Server IP\n", packet->siaddr);
    printf("%u : Client IP\n", packet->yiaddr);
    printf("%u : Transaction ID\n", packet->tran_ID);
    printf("%u : Lifetime\n", packet->lifetime);
}

