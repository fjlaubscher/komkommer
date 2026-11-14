#include "media_keys.h"

static void (*Step_Handler)(int direction);

void Media_Keys_Init(void (*step)(int direction))
{
    Step_Handler = step;
}

bool Media_Keys_Step(int direction)
{
    if (Step_Handler == nullptr) {
        return (false);
    }
    Step_Handler(direction);
    return (true);
}

#ifndef __APPLE__
void Media_Keys_Claim(bool, char const*)
{
}

bool Media_Keys_Claimed(void)
{
    return (false);
}
#endif
