/* Author: Tyler Cook
 * Date: 11 April, 2018
 * UNT CSCE 3530
 * Description:
*/

#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>
#include <arpa/inet.h>
#include <unistd.h>

typedef struct TCP_segment
{
    unsigned short int source;
    unsigned short int destination;
    unsigned int sequence;
    unsigned int ack;               //  11  12   13   14   15   16
    unsigned short int hdr_flags;   // URG, ACK, PSH, RST, SYN, FIN
    unsigned short int rec_window;
    unsigned short int checksum;
    unsigned short int urgent;
    unsigned int options;
}TCP_segment;

unsigned int computeChecksum(TCP_segment *tcp_seg);

void print_data(TCP_segment *seg);

int main(int argc, char *argv[])
{
    int listenFd, clientFd, n;                  // File descriptors and error checking
    struct sockaddr_in servaddr, addr;                // Server address structure
    int portNumber,destPort, sourcePort;                     // Port numbers
    TCP_segment *recBuff, *sendBuff;
    char buf[INET_ADDRSTRLEN];
    time_t t;
    socklen_t addr_len = sizeof(addr);
    unsigned int checksum;

    // Seed random number generator
    srand((unsigned int) time(&t));

    // Verify we have correct number of arguments
    if (argc != 2)
    {
        printf("Error: Program usage: %s port_number\n", argv[0]);
        exit(1);
    }
    else
    {
        portNumber = atoi(argv[1]);
    }

    // Initialize our listen socket
    listenFd = socket(AF_INET, SOCK_STREAM, 0);
    bzero(&servaddr, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_addr.s_addr = htons(INADDR_ANY);
    servaddr.sin_port = htons(portNumber);

    // Binds the above details to the socket
    bind(listenFd, (struct sockaddr *) &servaddr, sizeof(servaddr));

    // Start listening to incoming connections
    listen(listenFd, 10);
    printf("Listening for connection...\n");

    // Accept an incoming connection
    clientFd = accept(listenFd, (struct sockaddr*)NULL, NULL);
    if (clientFd < 0) // verify we accepted correctly
    {
        perror("ERROR on accept");
        exit(1);
    }
    printf("Successfully connected to client!\n\n");


    // Allocate memory for our incoming message
    recBuff = malloc(sizeof(TCP_segment));

    // Read message
    n = read(clientFd, recBuff, sizeof(TCP_segment));
    if (n < 0)
    {
        printf("Error reading from client\n");
    }

    // Print message
    printf("Recieved message from client:\n");
    print_data(recBuff);



    // Get the port number we're going to use for our outgoing messages
    n = getsockname(clientFd, (struct sockaddr *) &addr, &addr_len);
    if (n != 0)
    {
        printf("Error getting socket name\n");
    }
    else
    {
        inet_ntop(AF_INET, &addr, buf, sizeof(buf));
        sourcePort = ntohs(addr.sin_port);
    }

    // Get the port number for our connected client
    n = getpeername(clientFd,(struct sockaddr *) &addr, &addr_len);
    if (n != 0)
    {
        printf("Error getting peer name\n");
    }
    else
    {
        inet_ntop(AF_INET, &addr, buf, sizeof(buf));
        destPort = ntohs(addr.sin_port);
    }


    // Create connection granted TCP segment
    sendBuff = malloc(sizeof (struct TCP_segment));
    sendBuff->source = sourcePort;
    sendBuff->destination = destPort;
    sendBuff->sequence = rand();                      // Random initial server sequence number
    sendBuff->ack = recBuff->sequence + 1;            // Ack initial client sequence + 1
    sendBuff->hdr_flags = 0x0012;                     // SYN and ACK set to 1
    sendBuff->rec_window = 0;
    sendBuff->checksum = 0;
    sendBuff->urgent = 0;
    sendBuff->options = 0;

    // Compute and set the checksum for this header packet
    checksum = computeChecksum(sendBuff);
    sendBuff->checksum = checksum;

    // Write this packet to our socket
    printf("Sending Connection Granted TCP segment to client:\n");
    print_data(sendBuff);
    n = write(clientFd, sendBuff, sizeof(TCP_segment));


    // Read response from client
    n = read(clientFd, recBuff, sizeof(TCP_segment));
    if (n < 0)
    {
        printf("Error reading from client\n");
    }

    // Print message
    printf("Recieved acknowledgement segment from client:\n");
    print_data(recBuff);



    // Read close connection segment from client



    // Respond with ack


    // Send another close ack segment





    // Free our allocated memory
    free(recBuff);
    free(sendBuff);



    return 0;
}



unsigned int computeChecksum(TCP_segment *tcp_seg)
{
    unsigned short int cksum_arr[12];
    unsigned int i,sum=0, cksum, wrap;

    memcpy(cksum_arr, &tcp_seg, 24); //Copying 24 bytes

    for (i=0;i<12;i++)            // Compute sum
    {
        sum = sum + cksum_arr[i];
    }

    wrap = sum >> 16;             // Wrap around once
    sum = sum & 0x0000FFFF;
    sum = wrap + sum;

    wrap = sum >> 16;             // Wrap around once more
    sum = sum & 0x0000FFFF;
    cksum = wrap + sum;

    /* XOR the sum for checksum */

    cksum = 0xFFFF^cksum;
    return cksum;
}


void print_data(TCP_segment *seg)
{
    printf("\nTCP Segment Field Values\n");
    printf("0x%04X - Source Port Number\n", seg->source);
    printf("0x%04X - Destination Port Number\n", seg->destination);
    printf("0x%08X - Sequence Number\n", seg->sequence);
    printf("0x%08X - Ack Number\n", seg->ack);
    printf("0x%04X - Flags\n", seg->hdr_flags);
    printf("0x%04X - Rec Window\n", seg->rec_window);
    printf("0x%04X - Checksum\n", seg->checksum);
    printf("0x%04X - Urgent Pointer\n", seg->urgent);
    printf("0x%08X - Options\n\n", seg->options);
}
