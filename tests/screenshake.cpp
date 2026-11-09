#include "common/screenshake.h"
#include <cstdio>

int main()
{
    int failures = 0;
    auto check = [&](bool condition, char const* message) {
        if (!condition) {
            std::fprintf(stderr, "%s\n", message);
            failures++;
        }
    };

    ScreenShakeClass shake;
    check(shake.Offset(0) == 0, "Idle shake must have no offset");
    shake.Request(1, 1000);
    check(shake.Offset(1000) == 2, "Shake must start immediately without advancing time");
    check(shake.Offset(1015) == 2, "First visual step must last 16 ms");
    check(shake.Offset(1016) == -2, "Next visual step must reverse the offset");
    check(shake.Offset(1032) == 0, "Expired shake must restore an unshifted image");

    shake.Clear();
    shake.Request(1, 1000);
    shake.Request(1, 1010);
    check(shake.Offset(1063) != 0, "Overlapping requests must retain remaining duration");
    check(shake.Offset(1064) == 0, "Overlapping requests must eventually expire");
    shake.Request(1000000, 2000);
    check(shake.Offset(2383) != 0, "Large requests must still produce a brief visual shake");
    check(shake.Offset(2384) == 0, "Shake duration must be bounded");
    shake.Request(0, 3000);
    shake.Request(-1, 3000);
    check(shake.Offset(3000) == 0, "Zero/negative requests must not shake");

    shake.Request(1, UINT32_MAX - 15);
    check(shake.Offset(0) == -2, "Tick rollover must not interrupt a shake");
    check(shake.Offset(16) == 0, "Tick rollover must not leave a permanent shake");
    shake.Request(3, 4000);
    shake.Clear();
    check(shake.Offset(4000) == 0, "Video reset must cancel pending shake");
    return failures != 0;
}
