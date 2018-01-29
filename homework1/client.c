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
    char recvline[40960];
    struct sockaddr_in servaddr;

    /* AF_INET - IPv4 IP , Type of socket, protocol*/
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0)
    {
        perror("ERROR opening socket");
        exit(1);
    }
    bzero(&servaddr,sizeof(servaddr));

    servaddr.sin_family=AF_INET;
    servaddr.sin_port=htons(22000); // Server port number

    /* Convert IPv4 and IPv6 addresses from text to binary form */
    inet_pton(AF_INET,"129.120.151.94",&(servaddr.sin_addr));

    /* Connect to the server */
    connect(sockfd,(struct sockaddr *)&servaddr,sizeof(servaddr));

    while (n = read(sockfd, recvline, sizeof(recvline)) > 0)
    {
        printf("%s",recvline); // print the received text from server
    }

}
