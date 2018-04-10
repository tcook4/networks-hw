/* Author: Tyler Cook
 * Date: 11 April, 2018
 * UNT CSCE 3530
 * Description:
*/

#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <stdio.h>
#include <stdlib.h>             // Rand
#include <string.h>
#include <time.h>
#include <arpa/inet.h>      // inet_ntop()
#include <unistd.h>         // read() & write()

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
    int sockfd;                               // Socket file descriptor
    int n;                                   // Error checking for function calls
    struct sockaddr_in servaddr, addr;    // Server address structure
    int portNumber,destPort, sourcePort;                     // Port numbers
    TCP_segment *sendBuff, *readBuff;
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

    // AF_INET - IPv4 IP , Type of socket, protocol
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) // check for errors
    {
        perror("ERROR opening socket");
        exit(1);
    }
    bzero(&servaddr,sizeof(servaddr));
    servaddr.sin_family=AF_INET;
    servaddr.sin_port=htons(portNumber); // Server port number

    // Convert IPv4 address from text to binary form
    // inet_pton(AF_INET,"129.120.151.94",&(servaddr.sin_addr));
    inet_pton(AF_INET,"127.0.0.1",&(servaddr.sin_addr));

    // Connect to the server
    if(connect(sockfd,(struct sockaddr *)&servaddr,sizeof(servaddr)) < 0)
    {
        perror("Error connecting\n");
        exit(1);
    }

    // Get the port number we're going to use for our outgoing messages
    n = getsockname(sockfd, (struct sockaddr *) &addr, &addr_len);
    if (n != 0)
    {
        printf("Error getting socket name\n");
    }
    else
    {
        inet_ntop(AF_INET, &addr, buf, sizeof(buf));
        printf("Get socket name returns %s:%i!\n", buf, ntohs(addr.sin_port));
        sourcePort = ntohs(addr.sin_port);
    }

    // Get the port number for our connected server
    n = getpeername(sockfd,(struct sockaddr *) &addr, &addr_len);
    if (n != 0)
    {
        printf("Error getting peer name\n");
    }
    else
    {
        inet_ntop(AF_INET, &addr, buf, sizeof(buf));
        printf("Connection established successfully with %s:%i!\n", buf, ntohs(addr.sin_port));
        destPort = ntohs(addr.sin_port);

    }


    // Connection Request TCP Segment
    sendBuff = malloc(sizeof (struct TCP_segment));
    sendBuff->source = sourcePort;
    sendBuff->destination = destPort;
    sendBuff->sequence = rand();     // Random initial sequence number
    sendBuff->ack = 0;               // Ack 0
    sendBuff->hdr_flags = 0x0002;    // SYN set
    sendBuff->rec_window = 0;
    sendBuff->checksum = 0;
    sendBuff->urgent = 0;
    sendBuff->options = 0;

    // Compute and set the checksum for this header packet
    checksum = computeChecksum(sendBuff);
    sendBuff->checksum = checksum;

    // Write this packet to our socket
    printf("Sending connection request packet.\n");
    print_data(sendBuff);
    n = write(sockfd, sendBuff, sizeof(TCP_segment));
    if (n < 0)
    {
        printf("Error writing to server\n");
    }

    // Allocate memory for response
    readBuff = malloc(sizeof (struct TCP_segment));

    // Read response from server
    n = read(sockfd, readBuff, sizeof(TCP_segment));
    if (n < 0)
    {
        printf("Error reading from server\n");
    }
    printf("Recieved response from server:\n");
    print_data(readBuff);












    free(readBuff);
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

    cksum = 0xFFFF^cksum;
    return cksum;
}


void print_data(TCP_segment *seg)
{
    printf("TCP Segment Field Values\n");
    printf("0x%04X - Source Port Number\n", seg->source);
    printf("0x%04X - Destination Port Number\n", seg->destination);
    printf("0x%08X - Sequence Number\n", seg->sequence);
    printf("0x%08X - Ack Number\n", seg->ack);
    printf("0x%04X - Flags\n", seg->hdr_flags);
    printf("0x%04X - Rec Window\n", seg->rec_window);
    printf("0x%04X - Checksum\n", seg->checksum);
    printf("0x%04X - Urgent Pointer\n", seg->urgent);
    printf("0x%08X - Options\n", seg->options);
}
