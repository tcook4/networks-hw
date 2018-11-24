/* Author: Tyler Cook
 * Date: 25 April, 2018
 * UNT CSCE 3530
 * Description: This is the server for a client-server DHCP implementation. The server requests a gateway and subnet from the user
 * and then listens for requests. The server will receive a discover request, reply with an offer, and record the IP as allocated.
*/

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <math.h>

#define MAXSIZE 1000 // Max size of our stack

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

// Print the contents of a DHCP packet
void print_packet(struct dhcp_pkt *packet);


// Main method
int main(int argc, char **argv)
{
    struct sockaddr_in si_me, si_other, si_gate, si_sub;     // Socket structure
    int s, slen;                                             // Socket variables
    int portNumber;                                          // Port number to listen on
    dhcp_pkt *readBuff, *sendBuff;                           // DHCP Packet storage
    int validGate, validSub, validInput;                     // Gateway and Subnet Validation
    char gatewayIn[INET_ADDRSTRLEN];                         // Gateway input
    char subnetIn[INET_ADDRSTRLEN];                          // Subnet input
    unsigned int gateway, subnet;                            // Integer representation of IPs
    unsigned int range;                                      // Range of available IPs


    // Initialization
    validGate = 0;
    validSub = 0;
    validInput = 0;
    memset(gatewayIn, 0, INET_ADDRSTRLEN);
    memset(subnetIn, 0, INET_ADDRSTRLEN);
    gateway = 0;
    subnet = 0;
    slen = sizeof(si_other);


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

    while (!validInput)
    {
        // Get gateway from user and verify
        if (!validGate)
        {
            printf("Please enter gateway in dot format. Example: 192.168.0.1\n");
            scanf("%s", gatewayIn);
            if((inet_pton(AF_INET, gatewayIn, &(si_gate.sin_addr))) != 1)
            {
                printf("Error: Gateway not valid\n");
                continue;
            }
            validGate = 1;
        }

        // Get subnet from user and verify
        if (!validSub)
        {
            printf("Please enter subnet in dot format. Example: 255.255.255.0\n");
            scanf("%s", subnetIn);
            if((inet_pton(AF_INET, subnetIn, &(si_sub.sin_addr))) != 1)
            {
                printf("Error: Subnet not valid\n");
                continue;
            }
            validSub = 1;
        }
        break;
    }


    // Assign gateway and subnet
    gateway = ntohl(si_gate.sin_addr.s_addr);
    subnet = ntohl(si_sub.sin_addr.s_addr);

    // Total number of available IPs is inverse of subnet
    range = ~subnet;

    //create a UDP socket
    if ((s=socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP)) == -1)
    {
        die("Socket error");
    }

    // Zero out the structure
    memset((char *) &si_me, 0, sizeof(si_me));
    si_me.sin_family = AF_INET;
    si_me.sin_port = htons(portNumber);
    si_me.sin_addr.s_addr = htonl(INADDR_ANY);

    // Bind socket to given port mumber
    if( bind(s , (struct sockaddr*)&si_me, sizeof(si_me) ) == -1)
    {
        die("Bind error");
    }

    // Allocate memory for our messaages
    readBuff = malloc(sizeof(dhcp_pkt));
    sendBuff = malloc(sizeof(dhcp_pkt));

    // Run forever listening for data
    while(1)
    {
        memset(readBuff, 0, sizeof(dhcp_pkt));
        printf("Waiting for data...\n\n");
        fflush(stdout);

        // Recieve discover request from client
        if (recvfrom(s, readBuff, sizeof(dhcp_pkt), 0, (struct sockaddr *) &si_other, &slen) == -1)
        {
            die("recvfrom()");
        }

        // Print discover packet from client
        printf("Received DHCP Discover packet from client...\n\n");
        print_packet(readBuff);

        if (range == 0)
        {
            printf("ERROR: All available IPs are in use\n");
            continue;
        }

        // Create IP for new host
        gateway++;
        range--;

        // Create our response and print
        sendBuff->siaddr = readBuff->siaddr;
        sendBuff->yiaddr = htonl(gateway);    // Need to assign address here
        sendBuff->tran_ID = readBuff->tran_ID;
        sendBuff->lifetime = 3600;
        printf("Sendling DHCP Offer to client...\n\n");
        print_packet(sendBuff);

        // Send client offer
        if (sendto(s, sendBuff, sizeof(dhcp_pkt), 0, (struct sockaddr*) &si_other, slen) == -1)
        {
            die("sendto()");
        }

        // Flush output and listen for reques
        fflush(stdout);
        if (recvfrom(s, readBuff, sizeof(dhcp_pkt), 0, (struct sockaddr *) &si_other, &slen) == -1)
        {
            die("recvfrom()");
        }

        // Print received request packet
        printf("Received DHCP Request packet from client...\n\n");
        print_packet(readBuff);

        // Create ACK response and print
        sendBuff->siaddr = readBuff->siaddr;
        sendBuff->yiaddr = readBuff->yiaddr;
        sendBuff->tran_ID = readBuff->tran_ID;
        sendBuff->lifetime = readBuff->lifetime;
        printf("Sendling DHCP ACK to client...\n\n");
        print_packet(sendBuff);

        // Respond with ACK
        if (sendto(s, sendBuff, sizeof(dhcp_pkt), 0, (struct sockaddr*) &si_other, slen) == -1)
        {
            die("sendto()");
        }

        // Update our address structure
    }

    // Free memory
    free(readBuff);
    free(sendBuff);

    // Close the socket
    close(s);
    return 0;
}

// Print error and exit
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
