/* Server Code */

#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <stdio.h>
#include <string.h>
 
int main()
{
 
    char str[200];
    int listen_fd, conn_fd;
    struct sockaddr_in servaddr;
 
    /* AF_INET - IPv4 IP , Type of socket, protocol*/
    listen_fd = socket(AF_INET, SOCK_STREAM, 0);
 
    bzero(&servaddr, sizeof(servaddr));
 
    servaddr.sin_family = AF_INET;
    servaddr.sin_addr.s_addr = htons(INADDR_ANY);
    servaddr.sin_port = htons(22000);
 
    /* Binds the above details to the socket */
	bind(listen_fd,  (struct sockaddr *) &servaddr, sizeof(servaddr));
	/* Start listening to incoming connections */
	listen(listen_fd, 10);

    while(1)
    {
      /* Accepts an incoming connection */
      conn_fd = accept(listen_fd, (struct sockaddr*)NULL, NULL);
      bzero(str, 200);
      strcpy(str, "Hello world \n\r\0");
      write(conn_fd, str, strlen(str)); // write to the client
      close (conn_fd); //close the connection
    }
}
