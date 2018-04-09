/* Author: Tyler Cook
 * Date: 11 April, 2018
 * UNT CSCE 3530
 * Description:
*/

#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>


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

int main(int argc, char *argv[])
{

    int sockfd;                     // Socket file descriptor
    int n;                          // Error checking for read() and write() calls
    struct sockaddr_in servaddr;    // Server address structure
    int portNumber;                 // Port number
    TCP_segment *segBuff;


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


    // use getsockname ??

    segBuff = malloc(sizeof (struct TCP_segment));

    segBuff->source = 65234;
    segBuff->destination = 40234;
    segBuff->sequence = 1;
    segBuff->ack = 2;
    segBuff->hdr_flags = 0x2333;
    segBuff->rec_window = 0;
    segBuff->checksum = 0;  //Needs to be computed
    segBuff->urgent = 0;
    segBuff->options = 0;


    n = write(sockfd, segBuff, sizeof(TCP_segment));

    printf("Wrote %d bytes\n", n);

    free(segBuff);


    return 0;
}

