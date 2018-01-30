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

        printf("Reading %d bytes of data\n", bufferLength);

        // read the actual message
        bzero(buffer, 1024);
        do
        {
            n = read(conn_fd, buffer, bufferLength);
        }
        while (n < bufferLength);

        printf("Recieved data from client: \n");
        printf("%s\n", buffer);

/*        // parse our string and append it to our return
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

        */

        bzero(output, 1024);
        strcpy(output, buffer);

        printf("Writing to the client: \n");
        printf("%s", output);

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

        printf("Successfully sent message (hopefully!)\n");

    }
    //close (conn_fd); //close the connection
    return 0;
}
