#ifndef HARVESTPATH_H
#define HARVESTPATH_H

// Shared harvester ore selection for Tiberian Dawn and Red Alert.
//
// Include this from the game files. The walk that measures how far a cell
// really is stays in the game, because the two engines have their own
// passability rules. The rule that turns a list of ore cells into the one a
// harvester drives to is the same in both, so it lives here.
//
// The distance to a cell is the trip the harvester would really make and not
// the straight line between two cells. Ore behind a cliff is not as close as
// it looks on the map, so the walk decides which field the harvester drives
// to. The walk stores a negative cost for a cell it has not reached.
// HarvestTravel_Better is the rule that lets the first real step replace
// that, and it is the same in both engines.
//
// The work happens on plain numbers. The game converts to and from its own
// cell numbering.

enum
{
    // How many travel steps a harvester accepts over the nearest ore to get a
    // richer cell. The richest ore is then worth this many steps of walking.
    HARVEST_ORE_TOLERANCE = 3,

    // How many ore cells the picker is given at once. The window of cells
    // that can win is only a few steps wide, so the list stays short.
    HARVEST_ORE_CANDIDATES = 512
};

// One ore cell the harvester could drive to.
struct HarvestOre
{
    int Cell;   // The cell, in the caller's own numbering.
    int Cost;   // Travel distance from the harvester to the cell.
    int Amount; // Ore in the cell. A bigger number is richer ore.
};

// The index of the cell the harvester should drive to, or -1 when there is
// none.
//
// The nearest ore wins. Ore that is richer may be a few steps farther along
// the path, but not more than the price of that ore: HARVEST_ORE_TOLERANCE
// steps for the richest ore next to the closest one. A tie on the amount
// goes to the cell that is closer.
//
// The list holds the ore cells the harvester could drive to. Only the cells
// near the closest one can win, so the caller may narrow the list down to
// those before calling.
inline int HarvestOre_Pick(HarvestOre const* cells, int count)
{
    int nearest = -1;
    for (int index = 0; index < count; index++) {
        if (nearest < 0 || cells[index].Cost < nearest) {
            nearest = cells[index].Cost;
        }
    }
    if (nearest < 0) {
        return (-1);
    }

    int const limit = nearest + HARVEST_ORE_TOLERANCE;
    int best = -1;
    for (int index = 0; index < count; index++) {
        if (cells[index].Cost < nearest || cells[index].Cost > limit) {
            continue;
        }
        if (best < 0 || cells[index].Amount > cells[best].Amount
            || (cells[index].Amount == cells[best].Amount && cells[index].Cost < cells[best].Cost)) {
            best = index;
        }
    }
    return (best);
}

// True when `next` is a better travel cost than the one already stored.
//
// A cell the walk has not reached is stored as a negative number. The first
// real step onto that cell has to replace it. Comparing with `next >= stored`
// treats the sentinel as already closer than every real step, so the walk
// records nothing past the cell it started on. A harvester that has just
// emptied a tile then sees no ore and heads for the refinery with one tile
// of load.
inline bool HarvestTravel_Better(int stored, int next)
{
    return (stored < 0 || next < stored);
}

#endif
