/* Author: Tyler Cook
 * Date: 11 April, 2018
 * UNT CSCE 3530
 * Description: This program is the server to a client. This program simulates a TCP three way handshake, which is initiated
 * by the client and responded to by this server. After the handshake, the client initiates a close connection request. All
 * output is written to both the console and a file (server.out).
*/

#include <stdio.h>
#include <sys/socket.h>
#include <netdb.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>

// TCP Header Segment structure
typedef struct TCP_segment
{
    unsigned short int source;
    unsigned short int destination;
    unsigned int sequence;
    unsigned int ack;
    unsigned short int hdr_flags;
    unsigned short int rec_window;
    unsigned short int checksum;
    unsigned short int urgent;
    unsigned int options;
}TCP_segment;


// Compute the checksum for a TCP segment
unsigned int computeChecksum(TCP_segment *tcp_seg);

// Print data contained in a TCP segment
void print_data(TCP_segment *seg);

// Write data found in a TCP segment to a file
void write_data(TCP_segment *seg, int mode);


int main(int argc, char *argv[])
{
    int listenFd, clientFd, n;                  // File descriptors and error checking
    struct sockaddr_in servaddr, addr;          // Server address structure
    int portNumber,destPort, sourcePort;        // Port numbers
    TCP_segment *readBuff, *sendBuff;           // TCP Segment pointers
    time_t t;                                   // Time object for srand()
    socklen_t addr_len = sizeof(addr);          // Size of address struct
    unsigned int checksum;                      // Checksum storage


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

    while (1)
    {

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

        // Get the port number we're going to use for our outgoing messages
        n = getsockname(clientFd, (struct sockaddr *) &addr, &addr_len);
        if (n != 0)
        {
            printf("Error getting socket name\n");
        }
        else
        {
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
            destPort = ntohs(addr.sin_port);
        }

        // Allocate memory for our incoming and outgoing messages
        readBuff = malloc(sizeof(TCP_segment));
        sendBuff = malloc(sizeof (struct TCP_segment));
        if (readBuff == NULL || sendBuff == NULL)
        {
            printf("Malloc failure\n");
            exit(1);
        }


        // Connection request procedure
        // Read connection request message from client
        n = read(clientFd, readBuff, sizeof(TCP_segment));
        if (n < 0)
        {
            printf("Error reading from client\n");
        }

        // Print message
        printf("Received connection request from client:\n");
        print_data(readBuff);

        // Write this packet to file
        write_data(readBuff, 0);


        // Create connection granted TCP segment
        sendBuff->source = sourcePort;
        sendBuff->destination = destPort;
        sendBuff->sequence = rand();               // Random initial server sequence number
        sendBuff->ack = readBuff->sequence + 1;    // Ack initial client sequence + 1
        sendBuff->hdr_flags = 0x0012;              // SYN and ACK set to 1
        sendBuff->rec_window = 0;
        sendBuff->checksum = 0;
        sendBuff->urgent = 0;
        sendBuff->options = 0;

        // Compute and set the checksum
        checksum = computeChecksum(sendBuff);
        sendBuff->checksum = checksum;

        // Write this packet to our socket
        printf("Sending connection granted TCP segment to client:\n");
        print_data(sendBuff);
        n = write(clientFd, sendBuff, sizeof(TCP_segment));
        if (n < 0)
        {
            printf("Error writing to client\n");
        }

        // Write this packet to file
        write_data(sendBuff, 1);


        // Read ack from client
        n = read(clientFd, readBuff, sizeof(TCP_segment));
        if (n < 0)
        {
            printf("Error reading from client\n");
        }
        printf("Received acknowledgement segment from client:\n");
        print_data(readBuff);

        // Write this packet to file
        write_data(readBuff, 0);

        // Free allocated memory / prepare for close procedure
        free(readBuff);
        free(sendBuff);


        // Reallocate memory for close procedure
        readBuff = malloc(sizeof (struct TCP_segment));
        sendBuff = malloc(sizeof (struct TCP_segment));
        if (readBuff == NULL || sendBuff == NULL)
        {
            printf("Malloc failure\n");
            exit(1);
        }

        // Read close connection segment from client
        n = read(clientFd, readBuff, sizeof(TCP_segment));
        if (n < 0)
        {
            printf("Error reading from client\n");
        }
        printf("Received close request segment from client:\n");
        print_data(readBuff);

        // Write this packet to file
        write_data(readBuff, 0);

        // Create ACK for connection close request
        sendBuff->source = sourcePort;
        sendBuff->destination = destPort;
        sendBuff->sequence = 512;               // 512 per assignment instructions
        sendBuff->ack = readBuff->sequence + 1; // ACK is client sequence + 1
        sendBuff->hdr_flags = 0x0010;           // ACK set to 1
        sendBuff->rec_window = 0;
        sendBuff->checksum = 0;
        sendBuff->urgent = 0;
        sendBuff->options = 0;

        // Compute and set the checksum
        checksum = computeChecksum(sendBuff);
        sendBuff->checksum = checksum;

        // Send ACK for close request to client
        printf("Sending ACK for close request to client:\n");
        print_data(sendBuff);
        n = write(clientFd, sendBuff, sizeof(TCP_segment));
        if (n < 0)
        {
            printf("Error writing to client\n");
        }

        // Write this packet to file
        write_data(sendBuff, 1);


        // Create close acknowledgement segment
        sendBuff->sequence = 512;               // 512 per assignment instructions
        sendBuff->ack = readBuff->sequence + 1; // ACK is client sequence + 1
        sendBuff->hdr_flags = 0x0001;           // FIN set to 1
        sendBuff->checksum = 0;

        // Compute and set the checksum
        checksum = computeChecksum(sendBuff);
        sendBuff->checksum = checksum;

        // Send close request acknowledgement to client
        printf("Sending close request acknowledgement to client:\n");
        print_data(sendBuff);
        n = write(clientFd, sendBuff, sizeof(TCP_segment));
        if (n < 0)
        {
            printf("Error writing to client\n");
        }

        // Write this packet to file
        write_data(sendBuff, 1);


        // Read ack from client
        n = read(clientFd, readBuff, sizeof(TCP_segment));
        if (n < 0)
        {
            printf("Error reading from client\n");
        }
        printf("Received acknowledgement segment from client:\n");
        print_data(readBuff);

        // Write this packet to file
        write_data(readBuff, 0);


        // Free our allocated memory
        free(readBuff);
        free(sendBuff);

        // Close the connection
        close(clientFd);

        printf("Connection terminated.\n");

    }


    // Return
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
    fp = fopen("server.out", "a");

    // Mode indicates if we are recieving or sending this packet
    if (mode)
    {
        fprintf(fp, "Sending TCP header packet:\n");
    }
    else
    {
        fprintf(fp, "Received TCP header packet:\n");
    }

    // Write values to file
    fprintf(fp, "0x%04X - Source Port Number\n", seg->source);
    fprintf(fp, "0x%04X - Destination Port Number\n", seg->destination);
    fprintf(fp, "0x%08X - Sequence Number\n", seg->sequence);
    fprintf(fp, "0x%08X - Ack Number\n", seg->ack);
    fprintf(fp, "0x%04X - Flags\n", seg->hdr_flags);
    fprintf(fp, "0x%04X - Rec Window\n", seg->rec_window);
    fprintf(fp, "0x%04X - Checksum\n", seg->checksum);
    fprintf(fp, "0x%04X - Urgent Pointer\n", seg->urgent);
    fprintf(fp, "0x%08X - Options\n\n", seg->options);

    // Close the file
    fclose(fp);
}
