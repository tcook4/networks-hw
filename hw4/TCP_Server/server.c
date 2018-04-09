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


void computeChecksum();

void print_data(TCP_segment *seg);

int main(int argc, char *argv[])
{
    int listenFd, clientFd, n;                  // File descriptors and error checking
    struct sockaddr_in servaddr;                // Server address structure
    int portNumber;                             // Port number to use if supplied
    TCP_segment *segBuffer;

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
    printf("Successfully connected to client!\n");





    segBuffer = malloc(sizeof (struct TCP_segment));

    printf("sizeof after malloc is is %d\n", sizeof(TCP_segment));


    n = read(clientFd, segBuffer, sizeof(TCP_segment));

    printf("read %d bytes\n", n);


    print_data(segBuffer);



    free(segBuffer);



    return 0;
}



void computeChecksum()
{


    TCP_segment tcp_seg;
    unsigned short int cksum_arr[12];
    unsigned int i,sum=0, cksum, wrap;

    tcp_seg.source = 65234;
    tcp_seg.destination = 40234;
    tcp_seg.sequence = 1;
    tcp_seg.ack = 2;
    tcp_seg.hdr_flags = 0x2333;
    tcp_seg.rec_window = 0;
    tcp_seg.checksum = 0;  //Needs to be computed
    tcp_seg.urgent = 0;
    tcp_seg.options = 0;

    memcpy(cksum_arr, &tcp_seg, 24); //Copying 24 bytes

    printf ("TCP Field Values\n");
    printf("0x%04X\n", tcp_seg.source); // Printing all values
    printf("0x%04X\n", tcp_seg.destination);
    printf("0x%08X\n", tcp_seg.sequence);
    printf("0x%08X\n", tcp_seg.ack);
    printf("0x%04X\n", tcp_seg.hdr_flags);
    printf("0x%04X\n", tcp_seg.rec_window);
    printf("0x%04X\n", tcp_seg.checksum);
    printf("0x%04X\n", tcp_seg.urgent);
    printf("0x%08X\n", tcp_seg.options);

    printf ("\n16-bit values for Checksum Calculation\n");
    for (i=0;i<12;i++)            // Compute sum
    {
       printf("0x%04X\n", cksum_arr[i]);
       sum = sum + cksum_arr[i];
    }

    wrap = sum >> 16;             // Wrap around once
    sum = sum & 0x0000FFFF;
    sum = wrap + sum;

    wrap = sum >> 16;             // Wrap around once more
    sum = sum & 0x0000FFFF;
    cksum = wrap + sum;

    printf("\nSum Value: 0x%04X\n", cksum);
    /* XOR the sum for checksum */
    printf("\nChecksum Value: 0x%04X\n", (0xFFFF^cksum));


}


void print_data(TCP_segment *seg)
{
    printf("Source Port Number: %u\n", seg->source);
    printf("Destination Port Number: %u\n", seg->destination);
    printf("Sequence Number: %u\n", seg->sequence);
    printf("Ack Number: %u\n", seg->ack);
    printf("Flags: %u\n", seg->hdr_flags);
    printf("Rec Window: %u\n", seg->rec_window);
    printf("Checksum: %u\n", seg->checksum);
    printf("Urgent Pointer: %u\n", seg->urgent);
    printf("Options: %u\n", seg->options);
}
