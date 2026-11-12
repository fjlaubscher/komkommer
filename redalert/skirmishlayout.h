// Logical 320x200 coordinates, scaled by the standalone UI's resolution factor.
#ifndef RA_SKIRMISH_LAYOUT_H
#define RA_SKIRMISH_LAYOUT_H

namespace RASkirmish
{
namespace Layout
{
struct Rect
{
    int X;
    int Y;
    int Width;
    int Height;
};

// Dialog_Box reserves 15 pixels for each side's metal rail and 12 at the bottom.
// Keep a further two-pixel side inset rather than drawing on top of those rails.
static Rect const Dialog = {5, 3, 310, 197};
static Rect const Interior = {22, 19, 276, 169};
static Rect const Name = {52, 21, 76, 9};
static Rect const MapList = {22, 42, 112, 122};
static Rect const MapInfo = {22, 168, 112, 7};
static Rect const Roster = {142, 42, 156, 72};
static Rect const AddAI = {142, 117, 54, 9};
static Rect const Credits = {173, 130, 30, 7};
static Rect const Units = {255, 130, 25, 7};
static Rect const Tech = {173, 142, 30, 7};
static Rect const Difficulty = {264, 140, 34, 10};
static Rect const Rules = {142, 153, 74, 22};
static Rect const ExtraRules = {220, 153, 78, 22};
static Rect const Start = {250, 178, 48, 10};
static Rect const Defaults = {112, 178, 96, 10};
static Rect const Back = {22, 178, 48, 10};
static int const ColumnX[] = {164, 211, 257, 288};
static int const ColumnWidth[] = {45, 44, 29, 10};
enum { HeaderY = 34, RowPitch = 9 };
}
}
#endif
