#include <stdio.h>
#include "config.h"

int main() {
    RoomConfig* cfg;
    char* foundIP;
    size_t foundPort;

    cfg = rc_load("./rooms.cfg");
    foundIP = rc_getIP(cfg, 1);
    foundPort = rc_getPort(cfg, 1);
    printf("IP=%s\n", foundIP);
    printf("port=%ld\n", foundPort);
    
    rc_free(cfg);
    return 0;
}