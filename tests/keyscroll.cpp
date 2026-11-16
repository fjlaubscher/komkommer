#include "common/keyscroll.h"

#include <cstdio>

// The step table is fastest first, matching the edge-scroll tables. 100 is
// the slowest step and 900 the fastest, so a change in the ramp is obvious.

namespace
{
    int const Steps[KEYSCROLL_STEPS] = {900, 800, 700, 600, 500, 400, 300, 200, 100};

    KeyScrollPlan Step(KeyScrollState& state, int x, int y, bool held, int scroll_rate, bool scrolled)
    {
        KeyScrollPlan plan = KeyScroll_Plan(state, x, y, held, scroll_rate, true, Steps, KEYSCROLL_STEPS);
        KeyScroll_Commit(state, plan, scrolled);
        return (plan);
    }
}

int main()
{
    int failures = 0;
    auto check = [&](bool condition, char const* message) {
        if (!condition) {
            std::fprintf(stderr, "%s\n", message);
            failures++;
        }
    };

    int x = 0;
    int y = 0;
    bool held = false;
    KeyScroll_Axes(false, false, true, false, x, y, held);
    check(x == 0 && y == -1 && held, "up is north");
    check(KeyScroll_Direction(x, y) == 0, "north is direction 0");

    KeyScroll_Axes(true, false, true, false, x, y, held);
    check(x == -1 && y == -1 && held, "up and left are both held");
    check(KeyScroll_Direction(x, y) == (7 << 5), "up and left is northwest");

    KeyScroll_Axes(true, false, false, false, x, y, held);
    check(KeyScroll_Direction(x, y) == (6 << 5), "left is west");
    KeyScroll_Axes(false, true, false, true, x, y, held);
    check(KeyScroll_Direction(x, y) == (3 << 5), "right and down is southeast");

    KeyScroll_Axes(true, true, false, false, x, y, held);
    check(x == 0 && y == 0 && held, "left and right cancel and still count as held");

    KeyScrollState state;
    KeyScroll_Axes(false, false, true, false, x, y, held);
    KeyScrollPlan plan = Step(state, x, y, held, 3, true);
    check(plan.Distance == 100, "the first step is the slowest");
    check(plan.Direction == 0, "the first step faces the held key");
    check(state.Inertia == 1, "a successful step starts the ramp");

    plan = Step(state, x, y, held, 3, true);
    check(plan.Distance == 200, "the next tick is one step faster");
    check(state.Inertia == 2, "inertia climbs while the key is held");

    for (int tick = 0; tick < 20; tick++) {
        plan = Step(state, x, y, held, 3, true);
    }
    check(plan.Distance == 500, "scroll rate 3 caps at the middle step");
    check(plan.Distance != 900, "the cap stays short of the fastest step");

    state = KeyScrollState();
    for (int tick = 0; tick < 20; tick++) {
        plan = Step(state, x, y, held, 0, true);
    }
    check(plan.Distance == 800, "scroll rate 0 caps at the second step");

    state = KeyScrollState();
    for (int tick = 0; tick < 20; tick++) {
        plan = Step(state, x, y, held, 6, true);
    }
    check(plan.Distance == 200, "scroll rate 6 barely leaves the slowest step");

    state = KeyScrollState();
    plan = Step(state, x, y, held, 3, false);
    check(plan.Distance == 100, "a blocked view still asks for the slow step");
    check(state.Inertia == 0, "a blocked step does not speed up");

    state = KeyScrollState();
    for (int tick = 0; tick < 8; tick++) {
        plan = Step(state, x, y, held, 3, true);
    }
    check(plan.Distance == 500, "the ramp reaches the cap before the release");
    int const coast = state.Inertia;
    plan = Step(state, 0, 0, false, 3, true);
    check(plan.Distance == 500, "releasing the key coasts at the speed it had");
    check(state.Inertia == coast - 1, "coasting eases off by one step");

    state.Inertia = 4;
    state.Direction = 0;
    plan = Step(state, 0, 0, true, 3, true);
    check(plan.Distance == 0, "north and south held together do not coast");
    check(plan.Direction == KEYSCROLL_NONE, "a cancelling pair has no direction");
    check(state.Inertia == 3, "a cancelling pair eases the ramp down");

    state = KeyScrollState();
    plan = Step(state, 0, 0, false, 3, true);
    check(plan.Distance == 0 && state.Inertia == 0, "an idle view stays still");

    check(KeyScroll_Default(0, 80) == 80, "a saved 0 scroll key is the arrow default");
    check(KeyScroll_Default(22, 80) == 22, "a rebound scroll key is kept");

    state = KeyScrollState();
    for (int tick = 0; tick < 20; tick++) {
        plan = Step(state, x, y, held, 100, true);
    }
    check(plan.Distance == 100, "a scroll rate past the table stays on the slowest step");

    state = KeyScrollState();
    for (int tick = 0; tick < 20; tick++) {
        plan = Step(state, x, y, held, -1, true);
    }
    check(plan.Distance == 900, "a negative scroll rate stays inside the table");

    return (failures != 0);
}
