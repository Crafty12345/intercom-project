#include <signal.h>
#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdlib.h>
#include <sys/select.h>
#include <linux/input.h>
#include <time.h>
#include <pthread.h>
#include <string.h>
#include "config.h"

#define FILENAME "/dev/input/by-id/usb-Razer_Razer_Ornata_V3_X-if01-event-kbd"

int fp;
RoomConfig* cfg;
bool keyHeld;
time_t startTime;
bool timerRunning;

void onInterrupt() {
    close(fp);
    if (cfg != NULL) {
        rc_free(cfg);
    }
    printf("Program exiting\n");
    exit(SIGINT);
}

void *timerLoop(void* pArgs) {
    time_t stop;
    int threshold;
    int elapsedTime;
    
    keyHeld = true;
    timerRunning = true;
    threshold = 1;
    stop = time(NULL);
    elapsedTime = stop - startTime;
    while (elapsedTime < threshold) {
        stop = time(NULL);
        elapsedTime = stop - startTime;
        printf("Time elapsed: %d\n", threshold);
    }
    timerRunning = false;
    // terminate connection here
    keyHeld = false;
}

int main() {
    struct input_event event;
    char selectedIP[32];
    int selectedNum;
    time_t start, stop;
    int elapsedMsec;

    start = time(NULL);

    elapsedMsec = 0;

    cfg = rc_load("./rooms.cfg");


    fp = open(FILENAME, O_RDONLY);
    signal(SIGINT, onInterrupt);
    while (1) {
        read(fp, &event, sizeof(struct input_event));
        if (event.type == 1) {
            selectedNum = event.code - 1;
            if ((selectedNum >= 1) && (selectedNum <= 10)) {
                if (rc_containsRoom(cfg, selectedNum)) {
                    strcpy(selectedIP, rc_getIP(cfg, selectedNum));
                    keyHeld = true;
                    startTime = time(NULL);
                    if (!timerRunning) {
                        pthread_t thread;
                        pthread_create(&thread, NULL, timerLoop, NULL);
                    }
                    //printf("IP: %s\n", selectedIP);
                }
            }
        }

        // clock_t elapsed;
        // elapsed = clock() - before;
        // elapsedMsec = elapsed * 1000 / CLOCKS_PER_SEC;
        // if (elapsedMsec >= threshold) {
        //     printf("Key released");
        // }
    }

    rc_free(cfg);
    return 0;
}