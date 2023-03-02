#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>

void error(char *msg) {
    perror(msg);
    exit(1);
}

int main(void)  {
    int sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) error("ERROR opening socket");
    struct sockaddr_in serv_addr, cli_addr;
    bzero((char *) &serv_addr, sizeof(serv_addr));
    int portno = 8888;
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = INADDR_ANY;
    serv_addr.sin_port = htons(portno);
    if (bind(sockfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0)
        error("ERROR on binding");
    listen(sockfd,5);
    int clilen = sizeof(cli_addr);
    
    int newsockfd = accept(sockfd, (struct sockaddr *) &cli_addr, &clilen);
    if (newsockfd < 0)
        error("ERROR on accept");

    while (1) {
        char buffer[256];
        bzero(buffer,256);
        if (fork() != 0) {
            close(newsockfd);
            newsockfd = accept(sockfd, (struct sockaddr *) &cli_addr, &clilen);
            if (newsockfd < 0)
                error("ERROR on accept");
        } else {
            close(sockfd);
            dup2(newsockfd,0);
            dup2(newsockfd,1);
            int n = read(newsockfd,buffer,255);
            if (n < 0)
                error("ERROR reading from socket");
                
            //printf("Here is the message: %s\n",buffer);
            /*n = write(newsockfd,"I got your message\n",20);
            if (n < 0)
                error("ERROR writing to socket");*/

            char cmd[100] = "echo '";
            buffer[strcspn(buffer, "\n")] = 0;
            strcat(cmd,buffer);
            strcat(cmd,"' | ./output/toupper");

            system(cmd);
            close(newsockfd);
            return 0;
        }
    }
    return 0;
}