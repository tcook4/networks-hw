/* Written by Tyler Cook
 * UNT CSCE 3530
 * Homework 1
 * January 29th, 2019
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
    char buffer[1024];
    int listen_fd, conn_fd, n;
    struct sockaddr_in servaddr;
    char output[1024];
    int words, characters, i;
    words = 0;
    characters = 0;
    int dataLength;
    int bufferLength;
    int portNumber;
    char numBuff[50];

    if (argc == 2)
    {
        portNumber = atoi(argv[1]);
    }


    /* AF_INET - IPv4 IP , Type of socket, protocol*/
    listen_fd = socket(AF_INET, SOCK_STREAM, 0);

    bzero(&servaddr, sizeof(servaddr));

    servaddr.sin_family = AF_INET;
    servaddr.sin_addr.s_addr = htons(INADDR_ANY);
    if (argc == 2)
    {
        servaddr.sin_port = htons(portNumber);
    }
    else
    {
        servaddr.sin_port = htons(22000);
    }

    /* Binds the above details to the socket */
    bind(listen_fd,  (struct sockaddr *) &servaddr, sizeof(servaddr));
    /* Start listening to incoming connections */
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
    // word count and character count
    while(1)
    {
        // get input from client
        // get data size
        n = read(conn_fd, (char*)&dataLength, sizeof(dataLength));
        if (n < 0)
        {
            perror("Error getting data size\n");
        }

        // convert to buffer size
        bufferLength = ntohl(dataLength);

        // read the actual message
        bzero(buffer, 1024);
        n = read(conn_fd, buffer, bufferLength);
        if (n < 0)
        {
            perror("Error reading message from client\n");
        }

        bzero(output, 1024);
        // parse our string and append it to our return
        for (i = 0; i < strlen(buffer); i++)
        {
            if (buffer[i] == ' ')
            {
                words++;
                characters++;
                output[i] = ' ';
            }
            else if (buffer[i] >= 'A' && buffer[i] <= 'Z')
            {
                output[i] = buffer[i] + 32;
                characters++;
            }
            else
            {
                output[i] = buffer[i];
                characters++;
            }
        }

        // Generate our response information
        strcat(output, "\nTotal Characters: ");
        sprintf(numBuff, "%d", characters);
        strcat(output, numBuff);
        strcat(output, "\nTotal Words: ");
        sprintf(numBuff, "%d", words);
        strcat(output, numBuff);
        strcat(output, "\n\nEnd Transmission!\n\n");


        printf("Sending this to our client\n");
        printf("%s\n", output);

        strcpy(buffer, output);

        printf("Writing this after copy to the client: \n");
        printf("%s", buffer);

        // send client data size
        dataLength = htonl(output);
        n = write(conn_fd, (char*)&dataLength, sizeof(dataLength));
        if (n < 0)
        {
            perror("Error sending client data size\n");
        }

        // write to the client
        n = write(conn_fd, output, sizeof(output));
        if (n < 0)
        {
            perror("Error sending client message\n");
        }

        printf("Successfully sent message\n");

    }

    close (conn_fd); //close the connection
    return 0;
}
