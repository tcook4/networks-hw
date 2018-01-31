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
    int bufferLength;                     // Length of message to be sent
    int dataLength;                   // Length of message to be received
    int portNumber;                     // Port number to use if supplied
    char numBuff[12];                   // Number storage for casting int to string
    int space;                          // Multiple space detection

    // Initialization
    words = 0;
    characters = 0;
    space = 0;

    // AF_INET - IPv4 IP , Type of socket, protocol
    listen_fd = socket(AF_INET, SOCK_STREAM, 0);

    bzero(&servaddr, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_addr.s_addr = htons(INADDR_ANY);

    // Use our port number if given
    if (argc == 2)
    {
        portNumber = atoi(argv[1]);
        servaddr.sin_port = htons(portNumber);
    }
    else
    {
        servaddr.sin_port = htons(55122);
    }

    // Binds the above details to the socket
    bind(listen_fd,  (struct sockaddr *) &servaddr, sizeof(servaddr));

    // Start listening to incoming connections
    listen(listen_fd, 10);

    // Accepts an incoming connection
    conn_fd = accept(listen_fd, (struct sockaddr*)NULL, NULL);
    if (conn_fd < 0) // verify we accepted correctly
    {
        perror("ERROR on accept");
        exit(1);
    }

    // Main body loop
    // Receive message from user, strip capitalization and return new string along with
    // word and character count
    while(1)
    {
        // Get input from client
        // Get data size first
        n = read(conn_fd, (char*)&bufferLength, sizeof(bufferLength));
        if (n < 0)
        {
            perror("Error getting data size\n");
        }

        // Convert from network byte order
        dataLength = ntohl(bufferLength);

        // Read the actual message
        bzero(buffer, 1024);
        n = read(conn_fd, buffer, dataLength);
        if (n < 0)
        {
            perror("Error reading message from client\n");
        }

        // Parse our string for capitals, words and characters, and append it to our response
        // TODO: Multiple space detection
        // TODO: Punctuation detection
        bzero(output, 1024);
        for (i = 0; i < strlen(buffer); i++)
        {
            if (buffer[i] == ' ')
            {
                // skip multiple spaces
                if (space == 1)
                {
                    output[i] = ' ';
                    characters++;
                }
                else
                {
                    space = 1;
                    words++;
                    characters++;
                    output[i] = ' ';
                }
            }
            else if (buffer[i] >= 'A' && buffer[i] <= 'Z')
            {
                space = 0;
                output[i] = buffer[i] + 32;
                characters++;
            }
            else
            {
                space = 0;
                output[i] = buffer[i];
                characters++;
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
        dataLength = strlen(buffer);
        bufferLength = htonl(dataLength);
        n = write(conn_fd, (char*)&bufferLength, sizeof(bufferLength));
        if (n < 0)
        {
            perror("Error sending client data size\n");
        }

        printf("Testing - sending %d as the size\n", strlen(buffer));

        // Write actual message to the client
        n = write(conn_fd, buffer, strlen(buffer));
        if (n < 0)
        {
            perror("Error sending client message\n");
        }

        // Inform user of successful send and reset counters
        printf("Successfully sent message\n");
        characters = 0;
        words = 0;
    }

    close (conn_fd); //close the connection
    return 0;
}
