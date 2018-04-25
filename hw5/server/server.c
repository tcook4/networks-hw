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
#include <math.h>

#define BUFLEN 512  //Max length of buffer
#define MAXSIZE 1000 // Max size of our stack

typedef struct dhcp_pkt
{
    unsigned int siaddr;            // Server IP address
    unsigned int yiaddr;            // Client IP address
    unsigned int tran_ID;           // Transaction ID
    unsigned short int lifetime;    // Lease time
} dhcp_pkt;

// Function declarations, see below for descriptions

void die(char *s);

void print_packet(struct dhcp_pkt *packet);

int ERROR = 0;  // Error detected in expression

// Main method
int main(int argc, char **argv)
{
    struct sockaddr_in si_me, si_other, si_gate, si_sub;       // Socket structure
    int s, slen, i;                     // Socket variables
    char buf[BUFLEN];                                   // Recieve buffer
    int portNumber;                                     // Port number to listen on
    dhcp_pkt *readBuff, *sendBuff;
    int validGate, validSub, validInput;       // Gateway and Subnet Validation
    char gatewayIn[INET_ADDRSTRLEN];
    char subnetIn[INET_ADDRSTRLEN];
    unsigned int gateway, subnet;
    unsigned long timestamp;
    unsigned long *addresses;


    unsigned int available, range;
    struct sockaddr_in socktest;
    char ip4[INET_ADDRSTRLEN];


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

    int debug = 1;

    if (debug)
    {
        strncpy(gatewayIn, "192.168.1.1", 13);
        strncpy(subnetIn, "255.255.255.0", 13);
        inet_pton(AF_INET, gatewayIn, &(si_gate.sin_addr));
        inet_pton(AF_INET, subnetIn, &(si_sub.sin_addr));

        printf("Gate and sub are %s and %s\n", gatewayIn, subnetIn);
    }
    else
    {
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
    }

    // Assign gateway and subnet
    gateway = ntohl(si_gate.sin_addr.s_addr);
    subnet = ntohl(si_sub.sin_addr.s_addr);

    available = subnet && gateway;
    range = ~subnet;


    socktest.sin_addr.s_addr = htonl(available);
    inet_ntop(AF_INET, &(socktest.sin_addr), ip4, INET_ADDRSTRLEN);

    printf("The IPv4 address is: %s\n", ip4);
    printf("The range of addresses is %u\n", range);

    addresses = malloc(sizeof(unsigned long) * range);
    memset(addresses, 0, sizeof(unsigned long) * range);


    // Start address byte

    // End address byte



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
        bzero(buf, BUFLEN);
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

        // Create IP for new host
        gateway++;

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
