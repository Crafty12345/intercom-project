/****************************************************************
 * A TCP server that streams data to clients using sockets.
 * Each client connection evokes a child thread that sends the
 * data from a global circular buffer, which is written to by a
 * seperate thread. This thread (currently "record") can be adapted
 * to write any data that can be represented as a string of bytes.
 *
 * See Beej's Guide to Network Programming for a great tutorial
 *
 * -- Alex Spitzer 2013
 *
 ****************************************************************/

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include <pthread.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <time.h>

#include "new_client.h"
#include "capture.h"

#define PORT 9999
#define TARGET_ADDR "127.0.0.1"

int sockfd;
struct thread_data *td;
pthread_mutex_t sockMutex = PTHREAD_MUTEX_INITIALIZER;

void interrupt_handler(int signum)
{
    printf("Quitting server...\n");

    /* Clean up */
    close(sockfd);
    free(td->DATA);
    free(td->writePtr);
    free(td);
    exit(signum);
}

void *send_data(void *param)  // interprets void *param as a socket file descriptor
{
    int sockfd = *(int *) param;
    int readPtr = *td->writePtr;
    int rv;

    unsigned long bytes_to_send = td->CHUNK_SIZE * sizeof(td->DATA[0]);
    while (1) {
        pthread_mutex_lock(&td->mutex);
        while (readPtr == *td->writePtr) {  // wait for buffer to advance
            pthread_cond_wait(&td->cond, &td->mutex);
        }
        pthread_mutex_unlock(&td->mutex);
        pthread_mutex_lock(&sockMutex);
        if ((rv = send(sockfd, td->DATA + readPtr, bytes_to_send, MSG_NOSIGNAL)) == -1) {
            perror("send");
            close(sockfd);
            return NULL;
        }
        pthread_mutex_unlock(&sockMutex);

        if (rv != bytes_to_send) {
            fprintf(stderr, "Error: only sent %d out of %lu bytes", rv, bytes_to_send);
        }
        readPtr = (readPtr + td->CHUNK_SIZE) % td->BUFFER_SIZE;
    }
}



int main(int argc, char *argv[])
{
    const int CHUNK_SIZE = 128;
    const int BUFFER_SIZE = CHUNK_SIZE * 16;

    int connectionStatus;
    // struct addrinfo hints, *servinfo;
    // struct sockaddr_storage their_addr;
    struct sockaddr_in server;
    // socklen_t sin_size = sizeof their_addr;
    int yes = 1;

    connectionStatus = -1;

    // Open the socket
    if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
        perror("server: socket");
        return 1;
    }

    // Avoid address in use errors
    if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int)) == -1) {
        perror("setsockopt");
        return 1;
    }

    server.sin_family = AF_INET;
    
    server.sin_port = htons(PORT);
    server.sin_addr.s_addr = inet_addr(TARGET_ADDR);

    signal(SIGINT, interrupt_handler);  // Register the interrupt handler

    // Start the recording thread
    td = malloc(sizeof(struct thread_data));
    td->DATA = malloc(BUFFER_SIZE * sizeof(short));
    td->writePtr = malloc(sizeof(int));
    *td->writePtr = 0;
    td->CHUNK_SIZE = CHUNK_SIZE;
    td->BUFFER_SIZE = BUFFER_SIZE;

    pthread_mutex_init(&td->mutex, NULL);
    pthread_cond_init(&td->cond, NULL);

    pthread_t record_thread;
    pthread_create(&record_thread, NULL, record, td);

    while (1) {
        //pthread_mutex_lock(&sockMutex);
        connectionStatus = connect(sockfd, (struct sockaddr*)&server, sizeof server);
        //pthread_mutex_unlock(&sockMutex);
        while (connectionStatus == -1) {
            usleep(10000);
        }
        
        printf("connectionStatus=%d\n", connectionStatus);
        pthread_t serveThread;
        pthread_create(&serveThread, NULL, send_data, &sockfd);
        

        //new_fd = accept(sockfd, (struct sockaddr *)&their_addr, &sin_size);

        //printf("Connection from %s.\n", s);
        //pthread_t serve_thread;
        //pthread_create(&serve_thread, NULL, send_data, &new_fd);
    }

    close(sockfd);

    return 0; // who comes here?
}
