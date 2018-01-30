/* Written by Tyler Cook
 * UNT CSCE 3530
 * Homework 1
 * January 29th, 2019
 * Description: This program is the client side to a server. The user can send messages to the server
 * which parses the message and returns the word count and converts all uppercase characters to
 * lowercase.
 */


#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int main (int argc, char **argv)
{
    int sockfd, n;
    int len = sizeof(struct sockaddr);
    struct sockaddr_in servaddr;
    char buffer[1024];
    char input[1024];
    int iteration = 0;
    int dataLength;
    int bufferLength;

    // verify we have correct number of arguments
    if (argc != 2)
    {
        printf("Error: Program usage is %s port_number", argv[0]);
        exit(1);
    }

    // set our port number
    int portNumber = atoi(argv[1]);

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

    /* Convert IPv4 and IPv6 addresses from text to binary form */
    // using localhost for testing
    inet_pton(AF_INET,"127.0.0.1",&(servaddr.sin_addr));

    // Connect to the server
    if(connect(sockfd,(struct sockaddr *)&servaddr,sizeof(servaddr)) < 0)
    {
        perror("ERROR connecting!");
        exit(1);
    }

    printf("Successfully connected to server!\n");
    printf("Please enter a string to parse, or \"quit\" to exit\n");

    while (1)
    {
        // get input from user
        printf("Input: ");
        fgets(input, 1024, stdin);
        // check if we're quitting
        if (strcmp(input, "quit") == 0)
        {
            printf("Exiting...\n");
            bzero(buffer, 1024);
            strcpy(buffer, "quit");
            n = write(sockfd, buffer, sizeof(buffer));
            break;
        }

        // send data to server
        else
        {
            // zero our buffer and send the size
            bzero(buffer, 1024);
            strcpy(buffer, input);
            dataLength = strlen(buffer);

            printf("converting %d bytes to network order\n", dataLength);

            bufferLength = htonl(dataLength);
            n = write(sockfd, (char*)&bufferLength, sizeof(bufferLength));
            if (n < 0)
            {
                perror("Error sending message size\n");
            }

            // send the message
            write(sockfd, buffer, dataLength);
        }

        // listen for response
        // get response size
        n = read(sockfd, (char*)&bufferLength, sizeof(bufferLength));
        if (n < 0)
        {
            perror("Error receiving message size\n");
        }

        dataLength = ntohl(bufferLength);
        bzero(buffer, 1024);
        n = read(sockfd, buffer, bufferLength);
        if (n < 0)
        {
            perror("Error receiving message\n");
        }

        printf("%s\n", buffer);
        iteration++;
        printf("%d iterations\n", iteration);
        bzero(input, 1024);
    }
    return 0;
}
