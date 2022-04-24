#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <unistd.h>
#include <string.h>
#include <sys/socket.h>
#include <pthread.h>
#include <semaphore.h>
#include <arpa/inet.h>

#include "cachedata.h"

struct cachedata cdata;

struct llist
{
    struct llist *prev, *next;
    char key[10];
    char value[50];
};

struct llist *head = NULL;

struct thread_data
{
    struct cachedata *cdata;
    int connfd;
};

sem_t sem1, sem2;
pthread_t thread_id[50];
//pthread_t thread_id;
//pthread_t wthreads[50];
//pthread_t rthreads[50];
int count;

void *readfn(void *param)
{
    bool key_found = 0;
    struct thread_data tdata = *((struct thread_data *)param);
    if (head == NULL)
    {
        printf(" There is no data available\n");
        return -1;
    }
    else
    {
        //int current_value =0;
        //pthread_id_np_t tid;
        //tid = pthread_getthreadid_np();
        struct llist *temp;
        sem_wait(&sem1);
        for (temp = head; temp != NULL; temp = temp->next)
        {
            if (strcmp(temp->key, tdata.cdata->key) == 0)
            {
                strcpy(tdata.cdata->buff, temp->value);
                key_found = 1;
                break;
            }
        }
        sem_post(&sem1);
        if (key_found == 1)
        {
            send(tdata.connfd, tdata.cdata, sizeof(tdata.cdata), 0);

            printf("sent the data with key %s and  value is %s \n", tdata.cdata->key, tdata.cdata->buff);
        }
        else
        {
            printf("the key %s is not found in the server data\n", tdata.cdata->key);
        }

        pthread_exit(NULL);
    }
}

void *writefn(void *param)
{
    //int current_value =0;
    //pthread_id_np_t tid;
    //tid = pthread_getthreadid_np();

    printf("In writefn of server\n");

    struct thread_data tdata = *((struct thread_data *)param);
    struct llist *node = (struct llist *)malloc(sizeof(struct llist));
    struct llist *temp;
    strcpy(node->key, tdata.cdata->key);
    strcpy(node->value, tdata.cdata->buff);

    sem_wait(&sem1);

    if (head == NULL)
    {
        head = node;
        printf("head key is %s", head->key);
        node->next = NULL;
        node->prev = NULL;
    }
    else //traverse till end and add node
    {
        temp = head;
        while (temp != NULL)
        {
            if (temp->next != NULL)
            {
                temp = temp->next;
            }
            else
            {
                temp->next = node;
                node->prev = temp;
                node->next = NULL;
                temp = NULL;
            }
        }
    }

    sem_post(&sem1);
    bzero(&tdata, sizeof(tdata));

    printf("write thread executed and node updated in the list with key %s , value %s  \n", node->key, node->value);
    pthread_exit(NULL);
}

int main()
{
    int servsock, clientsock;
    struct sockaddr_in servaddr;
    struct sockaddr_storage serv_storage;
    socklen_t addr_size;

    sem_init(&sem1, 0, 1);
    sem_init(&sem2, 0, 1);

    if ((servsock = socket(AF_INET, SOCK_STREAM, 0)) < 0)
    {
        perror("Socket failed");
    }

    int optval = 1;
    if (setsockopt(servsock, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &optval, sizeof(optval)))
    {
        perror("setsockopt");
        //exit(-1);
    }

    servaddr.sin_family = AF_INET;
    servaddr.sin_addr.s_addr = INADDR_ANY;
    servaddr.sin_port = htons(8089);

    addr_size = sizeof(servaddr);

    if (bind(servsock, (struct sockaddr *)&servaddr, addr_size) < 0)
    {
        perror("bind failed");
    }
    else
    {
        printf("bind succesfull\n");
    }

    if (listen(servsock, 50) < 0)
    {
        perror("listen fail");
    }

    int i = 0, j = 0;
    while (1)
    {
        addr_size = sizeof(serv_storage);

        if ((clientsock = accept(servsock, (struct sockaddr *)&serv_storage, &addr_size)) < 0)
        {
            perror("accept");
        }
        else
        {
            printf("Accept succesfull");
        }

        //int option =0;
        // struct llist node;
        // if(head == NULL)
        // {
        //     head = &node;
        // }
        if (recv(clientsock, &cdata, sizeof(cdata), 0) < 0)
        {
            perror("receive error");
        }

        struct thread_data tdata;
        tdata.cdata = &cdata;
        tdata.connfd = clientsock;
        printf(" Request received is %c \n", tdata.cdata->data_request);
        if (cdata.data_request == 'r')
        {
            //read thread
            if (pthread_create(&thread_id[i++], NULL, readfn, &tdata) != 0)
            {
                perror("pthread_create failed for read");
            }
            else
            {
                printf("Created read thread\n");
            }
        }

        else if (cdata.data_request == 'w')
        {
            //write thread creation
            if (pthread_create(&thread_id[i++], NULL, writefn, &tdata) != 0)
            {
                perror("pthread_create failed for writefn");
            }
            else
            {
                printf("Created write thread\n");
            }
        }

        if (i >= 50)
        {
            printf("value of i s %d\n", i);
            i = 0;
            while (i < 50)
            {
                pthread_join(thread_id[i++], NULL);
            }

            //reset i to 0
            i = 0;
        }
    }
}
