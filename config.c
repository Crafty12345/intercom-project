#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "config.h"

RoomConfig* rc_load(char* pFilename) {
    FILE* fp;
    size_t numRooms;
    RoomConfig* result;
    //char temp[32];
    size_t temp;
    int i;

    fp = fopen(pFilename, "r");
    if (fp == NULL) {
        perror("Unable to load config file\n");
        return NULL;
    }

    fscanf(fp, "%ld=", &numRooms);
    result = rc_create(numRooms);
    for (i = 0; i < numRooms; i++) {
        printf("i=%d\n", i);
        fscanf(fp, "%ld=", &(result->entries[i].roomId));
        // num chars in IP + 1 (see comment in config.h)
        fscanf(fp, "%[^:]:", result->entries[i].ip);
        fscanf(fp, "%ld", &temp);
        printf("temp=%ld\n", temp);
        result->entries[i].port = temp;
    }

    return result;
}

RoomConfig* rc_create(size_t pNumRooms) {
    RoomConfig* result;
    result = malloc(sizeof(RoomConfig));
    result->numRooms = pNumRooms;
    result->entries = calloc(pNumRooms, sizeof(RoomConfig));
    // Initialise memory
    memset(result->entries, 0, sizeof(RoomConfig) * pNumRooms);
    return result;
}

bool rc_containsRoom(RoomConfig* pConfig, size_t pRoomID) {
    int i;
    bool found;

    i = 0;
    found = false;
    while ((i < pConfig->numRooms) && (!found)) {
        if (pConfig->entries[i].roomId == pRoomID) {
            found = true;
        }
        i++;
    }

    return found;
}

char* rc_getIP(RoomConfig* pConfig, size_t pRoomId) {
    int i;
    IPEntry currentEntry;
    size_t currentRoomID;
    char* result;

    i = 0;
    currentRoomID = 0;
    result = NULL;

    // linear search
    while ((i < pConfig->numRooms) && (result == NULL)) {
        currentEntry = pConfig->entries[i];
        currentRoomID = currentEntry.roomId;
        if (currentRoomID == pRoomId) {
            result = currentEntry.ip;
        }
        i++;
    }

    return result;
}

size_t rc_getPort(RoomConfig* pConfig, size_t pRoomId) {
    int i;
    IPEntry currentEntry;
    size_t currentRoomID;
    size_t result;
    bool found;

    i = 0;
    currentRoomID = 0;
    found = false;

    // linear search
    while ((i < pConfig->numRooms) && (!found)) {
        currentEntry = pConfig->entries[i];
        currentRoomID = currentEntry.roomId;
        if (currentRoomID == pRoomId) {
            result = currentEntry.port;
            found = true;
        }
        i++;
    }

    return result;
}


void rc_free(RoomConfig* pConfig) {
    free(pConfig->entries);
    pConfig->entries = NULL;
    free(pConfig);
    pConfig = NULL;
}