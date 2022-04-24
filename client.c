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
    memset(&request, 0, sizeof(request));
    close(sockdescriptor);
    pthread_exit(NULL);
}


void read_data()
{
    struct cachedata cdata;
    pthread_t tid;
    memset(&cdata, 0, sizeof(cdata));
    cdata.data_request = 'r';
    printf("Enter the key :");
    scanf("%10s", cdata.key);
    printf("key is %s and its size is %ld \n", cdata.key, wcslen(&cdata.key));

    pthread_create(&tid, NULL,
                   clientthread,
                   &cdata);

    pthread_join(tid, NULL);
}

void write_data()
{
    struct cachedata cdata;
    pthread_t tid;
    memset(&cdata, 0, sizeof(cdata));
    cdata.data_request = 'w';
    printf("Enter the key:");
    scanf("%10s", cdata.key);
    printf("Enter the data: ");
    scanf(" %50s", cdata.buff);
    printf("data in buffer is %s\n", cdata.buff);

    pthread_create(&tid, NULL,
                   clientthread,
                   &cdata);
    printf("Created the write thread\n");

    pthread_join(tid, NULL);
}

int main()
{
    printf("1. Read\n");
    printf("2. Write\n");

    while (1)
    {
        int choice;
        scanf("%d", &choice);
        switch (choice)
        {
        case 1:
        {
            read_data();
            break;
        }
        case 2:
        {
            write_data();
            break;
        }
        default:
            printf("Invalid Input\n");
            break;
        }
    }
}
