#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <pthread.h>

void error(const char *msg)
{
    perror(msg);
    exit(1);
}

void *handle_client(void *arg)
{
    int newsockfd = *(int *)arg;
    int n;
    char buffer[255];

    while (1)
    {
        memset(buffer, 0, sizeof(buffer));

        n = read(newsockfd, buffer, 255);
        if (n < 0)
            error("Error on reading");

        printf("Client:%s\n", buffer);

        if (strlen(buffer) >= 254)
        {
            fprintf(stderr, "Message too long\n");
            continue;
        }

        fgets(buffer, 255, stdin);

        n = write(newsockfd, buffer, strlen(buffer));
        if (n < 0)
            error("Error on writing. ");

        int i = strncmp("Bye", buffer, 3);
        if (i == 0)
            break;
    }

    close(newsockfd);

    pthread_exit(NULL);
}

int main(int argc, char *argv[])
{
    if (argc < 2)
    {
        fprintf(stderr, "port number not provided so the program terminated\n");
        exit(1);
    }

    int sockfd, newsockfd, portno;
    struct sockaddr_in serv_addr, cli_addr;
    socklen_t clilen;
    pthread_t thread_id;

    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0)
    {
        error("Error opening socket");
    }

    bzero((char *)&serv_addr, sizeof(serv_addr));
    portno = atoi(argv[1]);
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = INADDR_ANY;
    serv_addr.sin_port = htons(portno);

    if (bind(sockfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0)
        error("Binding Failed");

    listen(sockfd, 5);

    while (1)
    {
        clilen = sizeof(cli_addr);
        newsockfd = accept(sockfd, (struct sockaddr *)&cli_addr, &clilen);
        if (newsockfd < 0)
            error("Error on Accept");

        if (pthread_create(&thread_id, NULL, handle_client, (void *)&newsockfd) != 0)
            error("Error creating thread");
    }

    close(sockfd);
    return 0;
}
