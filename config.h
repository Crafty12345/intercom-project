#ifndef IP_ENTRY_H
#define IP_ENTRY_H
#include <stddef.h>
#include <stdbool.h>

// XXX.XXX.XXX.XXX = 15 chars
#define IP_NUM_CHARS 15

typedef struct ip_entry_t {
    size_t roomId;
    // Add 1 for null terminator
    char ip[IP_NUM_CHARS+1];
    size_t port;
} IPEntry;

typedef struct room_config_t {
    size_t numRooms;
    IPEntry* entries;
} RoomConfig;

RoomConfig* rc_load(char* pFilename);
RoomConfig* rc_create(size_t pNumRooms);
size_t rc_getPort(RoomConfig* pConfig, size_t pRoomId);
bool rc_containsRoom(RoomConfig* pConfig, size_t pRoomID);
char* rc_getIP(RoomConfig* pConfig, size_t pRoomID);
void rc_free(RoomConfig* pConfig);

#endif