#ifndef __MANGOS_CUSTOM_H
#define __MANGOS_CUSTOM_H

#include "Common.h"

class Custom
{
    public:
        #ifdef ENABLE_PLAYERBOTS
        static uint32 GetOnlineBotsCount();
        #endif
};

#endif
