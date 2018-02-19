/* Written by Tyler Cook
 * UNT CSCE 3530
 * Homework 1
 * January 29th, 2018
 * Description: This program is the server side to a client. The client sends messages which the server accepts,
 * then returns the word count and converts to message to lower case.
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
    int words, characters, i;           // Data storage for response message and iterator
    int bufferLength;                   // Length of message to be sent
    int dataLength;                     // Length of message to be received
    int portNumber;                     // Port number to use if supplied
    char numBuff[12];                   // Number storage for casting int to string
    int foundLetter;                    // Multiple space detection

    // Initialization
    words = 0;
    characters = 0;
    foundLetter = 0;

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
    // Receive message from user, strip capitalization and return new string along with
    // word and character count
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

        // Read the actual message
        bzero(buffer, 1024);
        n = read(conn_fd, buffer, dataLength);
        if (n < 0)
        {
            perror("Error reading message from client\n");
            exit(1);
        }

        // Check if we're quitting
        if (strcmp(buffer, "quit\n") == 0)
        {
            printf("Exiting...\n");
            break;
        }

        // Parse our string for capitals, words and characters, and append it to our response
        // Subtract one from strlen(buffer) to remove newline
        bzero(output, 1024);
        for (i = 0; i < strlen(buffer)-1; i++)
        {
            // Add spaces and prepare to add next found word
            if (buffer[i] == ' ')
            {
                foundLetter = 0;
                characters++;
                output[i] = ' ';
            }

            // Remove any capitalization
            else if (buffer[i] >= 'A' && buffer[i] <= 'Z')
            {
                if (foundLetter == 0) // Increment words if we're coming from a space
                {
                    words++;
                    foundLetter = 1;
                }
                output[i] = buffer[i] + 32;
                characters++;
            }

            // Add any lowercase as-is
            else if (buffer[i] >= 'a' && buffer[i] <= 'z')
            {
                if (foundLetter == 0)
                {
                    words++;
                    foundLetter = 1;
                }
                output[i] = buffer[i];
                characters++;
            }
            else
            {
                characters++;
                output[i] = buffer[i];
            }
        }

        // Generate the rest of our response information
        strcat(output, "\nTotal Characters: ");
        sprintf(numBuff, "%d", characters);
        strcat(output, numBuff);
        strcat(output, "\nTotal Words: ");
        sprintf(numBuff, "%d", words);
        strcat(output, numBuff);
        strcat(output, "\n");
        strcpy(buffer, output);

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

        // Inform user of successful send and reset counters
        printf("Successfully sent message\n");
        characters = 0;
        words = 0;
        foundLetter = 0;
    }

    close (conn_fd); // Close the connection
    return 0;
}
