#include <stdio.h>
#include "config.h"

int main() {
    RoomConfig* cfg;
    char* foundIP;

    cfg = rc_load("./rooms.cfg");
    foundIP = rc_getIP(cfg, 1);
    printf("IP=%s\n", foundIP);
    
    rc_free(cfg);
    return 0;
}