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
#include <linux/input.h>
#include <time.h>
#include <fcntl.h>

#include "client.h"
#include "capture.h"
#include "config.h"
#include "device_data.h"

#define PORT 9999

int sockfd;
struct thread_data *td;
pthread_mutex_t sockMutex = PTHREAD_MUTEX_INITIALIZER;
struct sockaddr_in server;
int keyboardFp;
char* targetIP;
RoomConfig* cfg;
bool keyHeld;
time_t startTime;
int numTimersRunning;
bool connectionClosed;
bool isReadingEvent;
struct input_event latestEvent;

void interrupt_handler(int signum)
{
    printf("Quitting server...\n");

    /* Clean up */
    close(sockfd);
    free(td->DATA);
    free(td->writePtr);
    free(td);
    if (cfg != NULL) {
        rc_free(cfg);
    }
    if (keyboardFp != 0) {
        close(keyboardFp);
    }
    exit(signum);
}

void *send_data(void *param)  // interprets void *param as a socket file descriptor
{
    //int sockfd = *(int *) param;
    int readPtr = *td->writePtr;
    int rv;

    unsigned long bytes_to_send = td->CHUNK_SIZE * sizeof(td->DATA[0]);
    while (isAnyKeyHeld()) {
        pthread_mutex_lock(&td->mutex);
        while (readPtr == *td->writePtr) {  // wait for buffer to advance
            pthread_cond_wait(&td->cond, &td->mutex);
        }
        pthread_mutex_unlock(&td->mutex);
        while (!isAnyKeyHeld()) {
            printf("Waiting for key to be pressed\n");
            usleep(10000);
        }
        pthread_mutex_lock(&sockMutex);
        if ((rv = send(sockfd, td->DATA + readPtr, bytes_to_send, MSG_NOSIGNAL)) == -1) {
            printf("Data thread: sockfd=%d\n", sockfd);
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
    return NULL;
}

bool isAnyKeyHeld() {
    return numTimersRunning > 0;
}

void* timerLoop(void* pArgs) {
    time_t stop;
    int threshold;
    int elapsedTime;
    // int yes;
    // yes = 1;

    numTimersRunning++;
    keyHeld = true;
    threshold = 2;
    stop = time(NULL);
    elapsedTime = stop - startTime;
    while (elapsedTime < threshold) {
        stop = time(NULL);
        elapsedTime = stop - startTime;
        //printf("Time elapsed: %d\n", threshold);
    }
    numTimersRunning--;
    return NULL;
}

void* getEventNonBlocking(void* pArg) {
    isReadingEvent = true;
    int keyboardFp = *(int*)pArg;
    read(keyboardFp, &latestEvent, sizeof(struct input_event));
    isReadingEvent = false;
    return NULL;
}

int main(int argc, char *argv[])
{
    const int CHUNK_SIZE = 128;
    const int BUFFER_SIZE = CHUNK_SIZE * 16;

    pthread_t serveThread;
    int connectionStatus;
    char selectedIP[32];
    int selectedRoom;
    numTimersRunning = 0;
    int yes = 1;

    connectionClosed = false;

    cfg = rc_load("./rooms.cfg");
    // defined in device_data.h
    keyboardFp = open(KB_FILENAME, O_RDONLY);
    if (keyboardFp == -1) {
        perror("Unable to open keyboard device");
    }

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
        if (!isReadingEvent) {
            pthread_t eventThread;
            eventThread = pthread_create(&eventThread, NULL, getEventNonBlocking, &keyboardFp);
        }
        if (latestEvent.type == 1) {
            selectedRoom = latestEvent.code - 1;
            if ((selectedRoom >= 1) && (selectedRoom <= 10)) {
                if (rc_containsRoom(cfg, selectedRoom)) {
                    //printf("Main thread: sockfd=%d\n", sockfd);
                    strcpy(selectedIP, rc_getIP(cfg, selectedRoom));
                    server.sin_addr.s_addr = inet_addr(selectedIP);
                    server.sin_port = htons(rc_getPort(cfg, selectedRoom));
                    connectionStatus = connect(sockfd, (struct sockaddr*)&server, sizeof server);
                    keyHeld = true;
                    startTime = time(NULL);
                    // TODO: timer limit
                    pthread_t timerThread;
                    pthread_create(&timerThread, NULL, timerLoop, NULL);

                    if (connectionStatus == 0) {
                        printf("connectionStatus=%d\n", connectionStatus);
                        pthread_create(&serveThread, NULL, send_data, NULL);
                    }
                }
            }
        }
        
    }

    close(sockfd);

    return 0; // who comes here?
}
