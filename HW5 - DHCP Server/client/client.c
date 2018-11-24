/* Author: Tyler Cook
 * Date: 25 April, 2018
 * UNT CSCE 3530
 * Description: This is the client for a client-server DHCP implementation. The client makes a discover request, recieives an
 * offer, requests the offered IP, then receives an ACK for that offer.
*/

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <time.h>

#define SERVER "129.120.151.94"      // CSCE Server
// #define SERVER "127.0.0.1"        // Local server

// DHCP packet structure
typedef struct dhcp_pkt
{
    unsigned int siaddr;            // Server IP address
    unsigned int yiaddr;            // Client IP address
    unsigned int tran_ID;           // Transaction ID
    unsigned short int lifetime;    // Lease time
} dhcp_pkt;


// Print error and exit
void die(char *s);

// Print the contents of a DHCP packet structure
void print_packet(struct dhcp_pkt *packet);


// Main method
int main(int argc, char **argv)
{
    struct sockaddr_in si_other, sock;  // Socket address structure
    int s, slen=sizeof(si_other);       // Sendto and recvfrom usage
    int portNumber, choice;             // Port number and user choice
    dhcp_pkt *readBuff, *sendBuff;      // Packet storage

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

    // Store server IP from given string
    if (inet_aton(SERVER , &si_other.sin_addr) == 0)
    {
        fprintf(stderr, "inet_aton() failed\n");
        exit(1);
    }

    // Run forever per assignment instructions
    while(1)
    {
        // Allocate memory for our messages
        readBuff = malloc(sizeof(dhcp_pkt));
        sendBuff = malloc(sizeof(dhcp_pkt));

        // Create DHCP discover packet
        inet_aton(SERVER, &sock.sin_addr);
        sendBuff->siaddr = sock.sin_addr.s_addr;
        sendBuff->yiaddr = 0;
        sendBuff->tran_ID = rand();
        sendBuff->lifetime = 0;

        // Print this packet before send
        printf("Sending DHCP Discover packet to server...\n\n");
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
        printf("Received DHCP Offer packet from server...\n\n");
        print_packet(readBuff);

        // Create DHCP request packet
        sendBuff->yiaddr = readBuff->yiaddr;
        sendBuff->tran_ID++;
        sendBuff->lifetime = readBuff->lifetime;

        // Print this packet before send
        printf("Sending DHCP Request packet to server...\n\n");
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
        printf("Received DHCP ACK packet from server...\n\n");
        print_packet(readBuff);

        // Free memory
        free(readBuff);
        free(sendBuff);

        // Allow user to get another IP address or quit
        do
        {
            printf("Connection completed\n");
            printf("1: Connect again\n");
            printf("2: Exit\n");
            printf("Choice: ");
            if ((scanf("%d", &choice)) != 1)
            {
                printf("Please only enter valid options\n");
                choice = 0;
            }

        } while (choice != 1 && choice != 2);

        if (choice == 2)
        {
            break;
        }
    }

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
    printf("Lifetime: %u\n\n", packet->lifetime);
}
