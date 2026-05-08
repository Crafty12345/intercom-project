
#ifndef THREAD_DATA
#define THREAD_DATA
#include <stdbool.h>


struct thread_data {
    short *DATA;
    int *writePtr;
    int CHUNK_SIZE;
    int BUFFER_SIZE;
    pthread_mutex_t mutex;
    pthread_cond_t cond;
};

void* getEventNonBlocking(void* pArg);
void* timerLoop(void* pArgs);
bool isAnyKeyHeld();
void *send_data(void *param);

#endif
