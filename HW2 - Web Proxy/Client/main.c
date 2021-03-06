/* Written by Tyler Cook
 * UNT CSCE 3530
 * Homework 2
 * March 2, 2018
 * Description: This program is the client side to a proxy server. The client takes a web address from a user
 * and passes it to the server. The server returns the web page if successful in connecting, or the HTTP response
 * if unsuccessful. Cached pages are returned if available.
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
    int bufferLength;               // Length of the buffer to be read to
    int portNumber;                 // Port number
    char* pos;                      // Remove newline from fgets
    uint32_t networkFileLength;     // Network order file length
    int fileLength;                 // Host order file length


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
    inet_pton(AF_INET,"129.120.151.94",&(servaddr.sin_addr));

    // Connect to the server
    if(connect(sockfd,(struct sockaddr *)&servaddr,sizeof(servaddr)) < 0)
    {
        perror("Error connecting\n");
        exit(1);
    }

    printf("Successfully connected to server!\n");
    printf("Please enter a web address.\n");

    // Main body loop
    // Ask user for input then display server response
    while (1)
    {
        // Initialize our buffers
        bzero(input, 1024);
        bzero(buffer, 1024);

        // Get input from user
        printf("Address: ");
        fgets(input, 1024, stdin);

        // Remove newline
        if ((pos=strchr(input, '\n')) != NULL)
        {
            *pos = '\0';
        }
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

        // Listen for server response
        printf("\nServer response: \n\n");

        // Read in size of response and convert to host order
        n = read(sockfd, &networkFileLength, sizeof(networkFileLength));
        if (n < 0)
        {
            perror("Error recieving message size from server:\n");
        }
        fileLength = ntohl(networkFileLength);

        // Print server response
        while (((n = read(sockfd, buffer, sizeof(buffer))) > 0) && (fileLength > 0))
        {
            printf("%s", buffer);
            fileLength -= n; // Decrement remaining bytes to be read

            // Check if we're done
            if (fileLength == 0)
            {
                break;
            }
        }

        printf("\n\nData successfully recieved\n");
    }

    return 0;
}
