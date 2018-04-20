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
    struct sockaddr_in si_me, si_other;                 // Socket structure
    int s, slen = sizeof(si_other), recv_len;           // Socket variables
    char buf[BUFLEN];                                   // Recieve buffer
    int portNumber;                                     // Port number to listen on
    dhcp_pkt *readBuff, *sendBuff;
    int ipTable[255];

    // Default messages
    char *quit = "quit";
    char *errMsg = "Invalid expression";

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
        printf("Waiting for data...\n");
        fflush(stdout);

        // Recieve discover request from client
        if (recvfrom(s, readBuff, sizeof(dhcp_pkt), 0, (struct sockaddr *) &si_other, &slen) == -1)
        {
            die("recvfrom()");
        }

        // Print discover packet from client
        printf("Received DHCP Discover packet from client\n");
        print_packet(readBuff);

        // Create our response
        sendBuff->siaddr = readBuff->siaddr;
        sendBuff->yiaddr = readBuff->yiaddr;    // Need to assign address here
        sendBuff->tran_ID = readBuff->tran_ID;
        sendBuff->lifetime = 3600;

        // Print packet before send
        printf("Sendling DHCP Offer to client\n");
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

        // Print request packet
        printf("Received DHCP Request packet from client\n");
        print_packet(readBuff);


        // Create ACK response
        sendBuff->siaddr = readBuff->siaddr;
        sendBuff->yiaddr = readBuff->yiaddr;
        sendBuff->tran_ID = readBuff->tran_ID;
        sendBuff->lifetime = readBuff->lifetime;


        // Print ACK before send
        printf("Sendling DHCP ACK to client\n");
        print_packet(sendBuff);

        // Respond with ACK
        if (sendto(s, sendBuff, sizeof(dhcp_pkt), 0, (struct sockaddr*) &si_other, slen) == -1)
        {
            die("sendto()");
        }



        // Reply to the client with our result
        if (sendto(s, buf, strlen(buf), 0, (struct sockaddr*) &si_other, slen) == -1)
        {
            die("sendto()");
        }
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
    printf("Lifetime: %u\n", packet->lifetime);
}
