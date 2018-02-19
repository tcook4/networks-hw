/* Written by Tyler Cook
 * UNT CSCE 3530
 * Homework 2
 * February 28th, 2018
 * Description:
 */

#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

int main(int argc, char **argv)
{
    char buffer[1024];                  // Communication buffer between client and server
    int listen_fd, conn_fd, n;          // File descriptors and error checking
    struct sockaddr_in servaddr;        // Server address structure
    char output[1024];                  // Output message storage
    int bufferLength;                   // Length of message to be sent
    int dataLength;                     // Length of message to be received
    int portNumber;                     // Port number to use if supplied

    FILE *fp;

    // Get allow list from file


    // Clear cache on startup



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
    listen_fd = socket(AF_INET, SOCK_STREAM, 0);
    bzero(&servaddr, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_addr.s_addr = htons(INADDR_ANY);
    servaddr.sin_port = htons(portNumber);

    // Binds the above details to the socket
    bind(listen_fd, (struct sockaddr *) &servaddr, sizeof(servaddr));

    // Start listening to incoming connections
    listen(listen_fd, 10);
    printf("Listening for connection...\n");

    // Accept an incoming connection
    conn_fd = accept(listen_fd, (struct sockaddr*)NULL, NULL);
    if (conn_fd < 0) // verify we accepted correctly
    {
        perror("ERROR on accept");
        exit(1);
    }

    printf("Successfully connected to client!\n");

    // Main body loop
    while(1)
    {
        // Get input from client
        // Get message size and convert from network order
        n = read(conn_fd, (char*)&bufferLength, sizeof(bufferLength));
        if (n < 0)
        {
            perror("Error getting data size\n");
            exit(1);
        }
        dataLength = ntohl(bufferLength);

        // Read the website address
        bzero(buffer, 1024);
        n = read(conn_fd, buffer, dataLength);
        if (n < 0)
        {
            perror("Error reading message from client\n");
            exit(1);
        }



        // Check if the user is allowed to access the web page





        // Check if we have a cached version of the webpage




        // If we have a cached version, foward it to the user




        // Else, go get the web page


        // Open a new socket


        // Get by hostname



        // Check if response code is 200
        // If 200, cashe and foward to user
        // Else, foward response

        // Get website info and put into list.txt






        // Send client the message size
        bufferLength = htonl(strlen(buffer));
        n = write(conn_fd, (char*)&bufferLength, sizeof(bufferLength));
        if (n < 0)
        {
            perror("Error sending client data size\n");
            exit(1);
        }

        // Write actual message to the client
        n = write(conn_fd, buffer, strlen(buffer));
        if (n < 0)
        {
            perror("Error sending client message\n");
            exit(1);
        }

    }

    close (conn_fd); // Close the connection
    return 0;
}
