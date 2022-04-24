#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <string.h>

#include <pthread.h>
#include "cachedata.h"

void *clientthread(void *param)
{
    printf("In client thread\n");
    struct cachedata request = *((struct cachedata *)param);
    int sockdescriptor;

    sockdescriptor = socket(AF_INET, SOCK_STREAM, 0);

    struct sockaddr_in servaddr;
    servaddr.sin_family = AF_INET;

    //inet_pton(AF_INET,"127.0.0.1", &servaddr.sin_addr); //!=0)
    //{
    //     perror("Error inet_aton");
    //     return;
    // }
    servaddr.sin_addr.s_addr = INADDR_ANY;
    servaddr.sin_port = htons(8089);

    if (connect(sockdescriptor, (struct sockaddr *)&servaddr, sizeof(servaddr)) < 0)
    {
        perror("Connect failed");
    }
    else
    {
        printf("connected to server\n");
    }

    send(sockdescriptor, &request, sizeof(request), 0);
    printf("data is Request %c , key %s  , buff %s  \n ", request.data_request, request.key, request.buff);
    printf(" Sent the data\n");
    bzero(&request, sizeof(request));
    close(sockdescriptor);
    pthread_exit(NULL);
}

int main()
{
    printf("1. Read\n");
    printf("2. Write\n");

    while (1)
    {
        int choice;
        scanf("%d", &choice);
        pthread_t tid;

        struct cachedata cdata;

        switch (choice)
        {
        case 1:
        {
            memset(&cdata,0,sizeof(cdata));
            cdata.data_request = 'r';
            printf("Enter the key :");
            scanf("%s", cdata.key);
            printf("key is %s and its size is %ld \n", cdata.key, strlen(cdata.key));


            pthread_create(&tid, NULL,
                           clientthread,
                           &cdata);


            pthread_join(tid, NULL);
            break;
        }
        case 2:
        {
            memset(&cdata,0,sizeof(cdata));
            cdata.data_request = 'w';
            printf("Enter the key:");
            scanf("%s", cdata.key);
            printf("Enter the data: ");
            scanf(" %s", cdata.buff);
            printf("\n");
            int len = strlen(cdata.buff);

            cdata.buff[len] = '\0';
            puts(cdata.buff);
            printf("data in buffer is %s\n", cdata.buff);

            pthread_create(&tid, NULL,
                           clientthread,
                           &cdata);
            printf("Created the write thread\n");
  
            pthread_join(tid, NULL);
            break;
        }
        default:
            printf("Invalid Input\n");
            break;
        }
    }
}
