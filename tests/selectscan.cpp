#include "common/selectscan.h"

#include <cstdio>

// Stand-ins for the ground layer and the aircraft heap. The real games pass
// Layer[LAYER_GROUND] and the Aircraft heap, which have the same two calls.
struct Unit
{
    int Id;
    SelectScanPoint Point;
    bool Accept;
    bool SelectSucceeds;
    bool Selected;
    int ScreenReads;

    bool Select(bool)
    {
        if (!SelectSucceeds) {
            return false;
        }
        Selected = true;
        return true;
    }
};

struct List
{
    Unit* Items[8];
    int N;

    int Count() const
    {
        return N;
    }
    Unit* operator[](int index)
    {
        return Items[index];
    }
    Unit* Ptr(int index)
    {
        return Items[index];
    }
};

int main()
{
    int failures = 0;
    auto check = [&](bool condition, char const* message) {
        if (!condition) {
            std::fprintf(stderr, "%s\n", message);
            failures++;
        }
    };

    // Edges are inside. One lepton past an edge is outside. The origin is not
    // always (0, 0): the view test subtracts the tactical coordinate first.
    SelectScanPoint origin = {100, 200};
    check(SelectScan_In_View(origin, 100, 200, 10, 20), "origin is on screen");
    check(SelectScan_In_View(SelectScanPoint{110, 200}, 100, 200, 10, 20), "right edge is on screen");
    check(SelectScan_In_View(SelectScanPoint{100, 220}, 100, 200, 10, 20), "bottom edge is on screen");
    check(SelectScan_In_View(SelectScanPoint{110, 220}, 100, 200, 10, 20), "far corner is on screen");
    check(!SelectScan_In_View(SelectScanPoint{99, 200}, 100, 200, 10, 20), "left of the view is off screen");
    check(!SelectScan_In_View(SelectScanPoint{111, 200}, 100, 200, 10, 20), "right of the view is off screen");
    check(!SelectScan_In_View(SelectScanPoint{100, 199}, 100, 200, 10, 20), "above the view is off screen");
    check(!SelectScan_In_View(SelectScanPoint{100, 221}, 100, 200, 10, 20), "below the view is off screen");

    Unit ground_match = {1, {100, 200}, true, true, false, 0};
    Unit ground_reject = {2, {100, 200}, false, true, false, 0};
    Unit ground_off = {3, {99, 200}, true, true, false, 0};
    Unit air_match = {4, {110, 220}, true, true, false, 0};
    Unit air_quiet = {5, {100, 200}, true, false, false, 0};
    List ground = {{&ground_match, &ground_reject, &ground_off}, 3};
    List aircraft = {{&air_match, &air_quiet}, 2};

    int seen[5];
    int seen_count = 0;
    bool saw_ground_as_air = false;
    bool saw_air_as_ground = false;
    bool read_screen_for_reject = false;
    bool allow_voice = true;

    Select_Matching_On_Screen(
        ground,
        aircraft,
        100,
        200,
        10,
        20,
        [&](Unit* unit, bool from_air_heap) -> bool {
            if (seen_count < 5) {
                seen[seen_count++] = unit->Id;
            }
            if (!from_air_heap && unit->Id >= 4) {
                saw_air_as_ground = true;
            }
            if (from_air_heap && unit->Id < 4) {
                saw_ground_as_air = true;
            }
            return unit->Accept;
        },
        [&](Unit* unit, bool) -> SelectScanPoint {
            unit->ScreenReads++;
            if (!unit->Accept) {
                read_screen_for_reject = true;
            }
            return unit->Point;
        },
        allow_voice);

    check(seen_count == 5, "every ground object and aircraft is offered to the filter");
    check(seen[0] == 1 && seen[1] == 2 && seen[2] == 3 && seen[3] == 4 && seen[4] == 5,
          "ground layer is scanned before the aircraft heap");
    check(!saw_ground_as_air && !saw_air_as_ground, "from_air_heap matches the collection");
    check(!read_screen_for_reject, "a rejected object does not get a screen test");
    check(ground_reject.ScreenReads == 0, "enemy, cloaked, or building stand-ins are not measured");
    check(ground_off.ScreenReads == 1 && !ground_off.Selected, "an off-screen match is measured and not selected");
    check(ground_match.Selected && air_match.Selected, "on-screen matches from both collections are selected");
    check(!air_quiet.Selected, "a Select that returns false does not mark the object selected");
    check(!allow_voice, "the first successful Select suppresses further response voice");

    // A failed Select must not turn the voice back on, and must not clear it
    // when nothing has been selected yet.
    allow_voice = true;
    Unit only_fail = {6, {100, 200}, true, false, false, 0};
    List one = {{&only_fail}, 1};
    List none = {};
    Select_Matching_On_Screen(
        one,
        none,
        100,
        200,
        10,
        20,
        [&](Unit* unit, bool) -> bool { return unit->Accept; },
        [&](Unit* unit, bool) -> SelectScanPoint { return unit->Point; },
        allow_voice);
    check(allow_voice, "a failed Select leaves the voice enabled");

    return failures != 0;
}
