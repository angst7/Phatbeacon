#ifndef CUSTOM_BOARD_H
#define CUSTOM_BOARD_H

// Include custom device support here

#if defined(BOARD_BEACON_BUDDY)
    #include "beaconbuddy.h"
#elif defined(BOARD_RUUVITAG)
    #include "ruuvitag.h"
#else
    #error "Board not defined"
#endif

#endif