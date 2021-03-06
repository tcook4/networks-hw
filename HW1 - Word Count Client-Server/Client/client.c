/* Written by Tyler Cook
 * UNT CSCE 3530
 * Homework 1
 * January 29th, 2018
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
    int sockfd;                     // Socket file descriptor
    int n;                          // Error checking for read() and write() calls
    struct sockaddr_in servaddr;    // Server address structure
    char buffer[1024];              // Sent and received message buffer
    char input[1024];               // fgets() input buffer
    int dataLength;                 // Length of the data to be sent
    int bufferLength;               // Length of the buffer to be read to
    int portNumber;                 // Port number
    int quit;                       // Quitting detection

    // Initialization
    quit = 0;

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

    // Convert IPv4 and IPv6 addresses from text to binary form
    inet_pton(AF_INET,"129.120.151.94",&(servaddr.sin_addr));

    // Connect to the server
    if(connect(sockfd,(struct sockaddr *)&servaddr,sizeof(servaddr)) < 0)
    {
        perror("Error connecting\n");
        exit(1);
    }

    printf("Successfully connected to server!\n");
    printf("Please enter a string to parse, or \"quit\" to exit\n");

    // Main body loop
    // Ask user for input then display server response
    while (1)
    {
        // Get input from user
        printf("Input: ");
        bzero(input, 1024);
        fgets(input, 1024, stdin);

        // Check if we're quitting
        if (strcmp(input, "quit\n") == 0)
        {
            quit = 1;
        }

        // Send data to server
        // Prepare our buffer
        bzero(buffer, 1024);
        strcpy(buffer, input);

        // Find and send the size of the message after converting to network order
        bufferLength = htonl(strlen(buffer));
        n = write(sockfd, (char*)&bufferLength, sizeof(bufferLength));
        if (n < 0)
        {
            perror("Error sending message size\n");
        }

        // Send the actual message body
        n = write(sockfd, buffer, strlen(buffer));
        if (n < 0)
        {
            perror("Error sending message\n");
        }

        // We can quit now
        if (quit == 1)
        {
            printf("Exiting...\n");
            break;
        }

        // Listen for server response
        // Receive response size and convert from network order
        n = read(sockfd, (char*)&bufferLength, sizeof(bufferLength));
        if (n < 0)
        {
            perror("Error receiving message size\n");
        }
        dataLength = ntohl(bufferLength);

        // Zero our buffer and read the message body
        bzero(buffer, 1024);
        n = read(sockfd, buffer, dataLength);
        if (n < 0)
        {
            perror("Error receiving message\n");
        }

        // Print server response to the user
        printf("Server response: \n");
        printf("%s\n", buffer);
    }

    return 0;
}
