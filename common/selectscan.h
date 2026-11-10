#ifndef SELECTSCAN_H
#define SELECTSCAN_H

// Standalone double-click selection. Include this from each game's display.cpp
// inside #ifndef REMASTER_BUILD. Remastered never compiles it; GlyphX does its
// own selection.
//
// The two games do not share TechnoClass, so this header does not name those
// types. Select_Matching_On_Screen only walks the collections. The game passes
// two predicates:
//   accept(object, from_air_heap)       — type, owner, cloak, and the rest
//   screen_point(object, from_air_heap) — where that object is drawn
//
// from_air_heap is false for the ground layer and true for the aircraft heap.
// A landed aircraft can be in both. The ground pass must keep using its own
// coordinate, which is why the flag exists.

// Lepton X/Y already taken with the game's Coord_X / Coord_Y.
struct SelectScanPoint
{
    int X;
    int Y;
};

// The tactical view includes its far edges. The old test was
// `xoff < 0 || xoff > width`, so 0 and width are both on screen.
inline bool SelectScan_In_View(SelectScanPoint point, int tac_x, int tac_y, int width, int height)
{
    int xoff = point.X - tac_x;
    int yoff = point.Y - tac_y;
    return xoff >= 0 && xoff <= width && yoff >= 0 && yoff <= height;
}

// One object. Filters run before the screen test, then Select(true).
// The first successful Select clears allow_voice so a whole group only
// acknowledges once. A later failed Select leaves that flag alone.
template <class Object, class Accept, class ScreenPoint>
void Select_Matching_One(Object* obj,
                         bool from_air_heap,
                         int tac_x,
                         int tac_y,
                         int width,
                         int height,
                         Accept accept,
                         ScreenPoint screen_point,
                         bool& allow_voice)
{
    if (!accept(obj, from_air_heap)) {
        return;
    }
    if (!SelectScan_In_View(screen_point(obj, from_air_heap), tac_x, tac_y, width, height)) {
        return;
    }

    bool old_allow_voice = allow_voice;
    if (obj->Select(true)) {
        old_allow_voice = false;
    }
    allow_voice = old_allow_voice;
}

// Ground layer first, then the aircraft heap. Buildings are not removed here:
// the ground layer also holds terrain and structures, and accept() rejects those.
// Red Alert vessels are ground-layer technos, so they are visited.
template <class GroundLayer, class AircraftHeap, class Accept, class ScreenPoint>
void Select_Matching_On_Screen(GroundLayer& ground,
                               AircraftHeap& aircraft,
                               int tac_x,
                               int tac_y,
                               int width,
                               int height,
                               Accept accept,
                               ScreenPoint screen_point,
                               bool& allow_voice)
{
    for (int index = 0; index < ground.Count(); index++) {
        Select_Matching_One(ground[index], false, tac_x, tac_y, width, height, accept, screen_point, allow_voice);
    }
    for (int index = 0; index < aircraft.Count(); index++) {
        Select_Matching_One(aircraft.Ptr(index), true, tac_x, tac_y, width, height, accept, screen_point, allow_voice);
    }
}

#endif
