/* Author: Tyler Cook
 * Date: 11 April, 2018
 * UNT CSCE 3530
 * Description:
*/

#include <sys/socket.h>
#include <netdb.h>
#include <stdio.h>
#include <stdlib.h>             // Rand
#include <string.h>
#include <time.h>
#include <arpa/inet.h>      // inet_pton()
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

void write_data(TCP_segment *seg, int mode);

int main(int argc, char *argv[])
{
    int sockfd, n;                          // Socket file descriptor and error checking
    struct sockaddr_in servaddr, addr;      // Server address structure
    int portNumber,destPort, sourcePort;    // Port number storage
    TCP_segment *sendBuff, *readBuff;       // TCP Segment storage for send and recieve
    time_t t;                               // Time variable
    socklen_t addr_len = sizeof(addr);      // Size of an address
    unsigned int checksum;                  // Checksum storage


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
        destPort = ntohs(addr.sin_port);
    }

    // Allocate memory for message and response
    readBuff = malloc(sizeof (struct TCP_segment));
    sendBuff = malloc(sizeof (struct TCP_segment));
    if (readBuff == NULL || sendBuff == NULL)
    {
        printf("Malloc failure\n");
        exit(1);
    }

    // Create connection Request TCP Segment
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
    printf("Sending connection request packet:\n");
    print_data(sendBuff);
    n = write(sockfd, sendBuff, sizeof(TCP_segment));
    if (n < 0)
    {
        printf("Error writing to server\n");
    }

    // Write this packet to file
    write_data(sendBuff, 1);

    // Read response from server
    n = read(sockfd, readBuff, sizeof(TCP_segment));
    if (n < 0)
    {
        printf("Error reading from server\n");
    }
    printf("Recieved response from server:\n");
    print_data(readBuff);

    // Write this packet to file
    write_data(readBuff, 0);


    // Create Acknowledgement TCP Segment
    sendBuff->sequence++;
    sendBuff->ack = readBuff->sequence+1;
    sendBuff->hdr_flags = 0x0010;           // ACK set to 1
    sendBuff->checksum = 0;

    // Compute and set checksum
    checksum = computeChecksum(sendBuff);
    sendBuff->checksum = checksum;

    // Print ack packet and send to server
    printf("Sending acknowledgement segment.\n");
    print_data(sendBuff);
    n = write(sockfd, sendBuff, sizeof(TCP_segment));
    if (n < 0)
    {
        printf("Error writing to server\n");
    }

    // Write this packet to file
    write_data(sendBuff, 1);

    // Connection process complete - free allocated memory
    free(readBuff);
    free(sendBuff);

    // Close connection procedure
    // Reallocate memory for close procedure
    readBuff = malloc(sizeof (struct TCP_segment));
    sendBuff = malloc(sizeof (struct TCP_segment));
    if (readBuff == NULL || sendBuff == NULL)
    {
        printf("Malloc failure\n");
        exit(1);
    }

    // Send close request TCP segment
    sendBuff->source = sourcePort;
    sendBuff->destination = destPort;
    sendBuff->sequence = 1024;       // 1024 per assignment instructions
    sendBuff->ack = 512;             // 512 per assignment instructions
    sendBuff->hdr_flags = 0x0001;    // FIN set
    sendBuff->rec_window = 0;
    sendBuff->checksum = 0;
    sendBuff->urgent = 0;
    sendBuff->options = 0;

    // Compute and set checksum
    checksum = computeChecksum(sendBuff);
    sendBuff->checksum = checksum;

    // Print packet and send to server
    printf("Sending close request segment.\n");
    print_data(sendBuff);
    n = write(sockfd, sendBuff, sizeof(TCP_segment));
    if (n < 0)
    {
        printf("Error writing to server\n");
    }

    // Write this packet to file
    write_data(sendBuff, 1);


    // Read acknowledgement from server
    n = read(sockfd, readBuff, sizeof(TCP_segment));
    if (n < 0)
    {
        printf("Error reading from server\n");
    }
    printf("Recieved ack from server:\n");
    print_data(readBuff);

    // Write this packet to file
    write_data(readBuff, 0);


    // Read close acknowledgement
    n = read(sockfd, readBuff, sizeof(TCP_segment));
    if (n < 0)
    {
        printf("Error reading from server\n");
    }
    printf("Recieved close acknowledgement from server:\n");
    print_data(readBuff);

    // Write this packet to file
    write_data(readBuff, 0);


    // Ack close acknowledgement
    // Create ack packet
    sendBuff->sequence++;
    sendBuff->ack = readBuff->sequence+1;
    sendBuff->hdr_flags = 0x0010;    // ACK set
    sendBuff->checksum = 0;

    // Compute and set checksum
    checksum = computeChecksum(sendBuff);
    sendBuff->checksum = checksum;

    // Send ack to server
    printf("Sending ack for close acknowledgement segment.\n");
    print_data(sendBuff);
    n = write(sockfd, sendBuff, sizeof(TCP_segment));
    if (n < 0)
    {
        printf("Error writing to server\n");
    }

    // Write this packet to file
    write_data(sendBuff, 1);


    // Free memory
    free(readBuff);
    free(sendBuff);

    return 0;
}


// Compute the checksum for a TCP segment pointer
unsigned int computeChecksum(TCP_segment *tcp_seg)
{
    unsigned short int cksum_arr[12];
    unsigned int i,sum=0, cksum, wrap;

    memcpy(cksum_arr, &tcp_seg, 24);

    // Compute the sum
    for (i=0;i<12;i++)
    {
        sum = sum + cksum_arr[i];
    }

    // Wrap around once
    wrap = sum >> 16;
    sum = sum & 0x0000FFFF;
    sum = wrap + sum;

    // Wrap around once more
    wrap = sum >> 16;
    sum = sum & 0x0000FFFF;
    cksum = wrap + sum;

    // One's complement
    cksum = 0xFFFF^cksum;
    return cksum;
}

// Print the fields for a TCP segment
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
    printf("0x%08X - Options\n\n\n", seg->options);
}

// Write TCP segment data to a file
void write_data(TCP_segment *seg, int mode)
{
    FILE *fp;   // FILE pointer

    // Open file in append mode
    fp = fopen("client.out", "a");

    // Mode indicates if we are recieving or sending this packet
    if (mode)
    {
        fprintf(fp, "Sending TCP header packet:\n");
    }
    else
    {
        fprintf(fp, "Recieved TCP header packet:\n");
    }

    fprintf(fp, "0x%04X - Source Port Number\n", seg->source);
    fprintf(fp, "0x%04X - Destination Port Number\n", seg->destination);
    fprintf(fp, "0x%08X - Sequence Number\n", seg->sequence);
    fprintf(fp, "0x%08X - Ack Number\n", seg->ack);
    fprintf(fp, "0x%04X - Flags\n", seg->hdr_flags);
    fprintf(fp, "0x%04X - Rec Window\n", seg->rec_window);
    fprintf(fp, "0x%04X - Checksum\n", seg->checksum);
    fprintf(fp, "0x%04X - Urgent Pointer\n", seg->urgent);
    fprintf(fp, "0x%08X - Options\n\n", seg->options);

    fclose(fp);
}
