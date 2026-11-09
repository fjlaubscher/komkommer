#ifndef SCREENSHAKE_H
#define SCREENSHAKE_H

#include <cstdint>

// Presentation-only state. Time is supplied by the renderer; this never
// advances or delays simulation, consumes game RNG, or retains engine objects.
class ScreenShakeClass
{
public:
    ScreenShakeClass()
        : Start(0)
        , Duration(0)
    {
    }

    void Clear()
    {
        Duration = 0;
    }

    void Request(int shakes, uint32_t now)
    {
        if (shakes <= 0) {
            return;
        }
        uint32_t elapsed = now - Start;
        uint32_t remaining = elapsed < Duration ? Duration - elapsed : 0;
        uint32_t extra = (shakes > 12 ? 12 : shakes) * 32;
        Duration = remaining + extra;
        if (Duration > 384) {
            Duration = 384;
        }
        Start = now;
    }

    int Offset(uint32_t now) const
    {
        uint32_t elapsed = now - Start;
        return elapsed < Duration ? ((elapsed / 16) % 2 ? -2 : 2) : 0;
    }

private:
    uint32_t Start;
    uint32_t Duration;
};

#endif
