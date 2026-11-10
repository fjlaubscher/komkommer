#ifndef PRODUCTIONQUEUE_H
#define PRODUCTIONQUEUE_H

#include <cstddef>

// One queued production request, shared by Tiberian Dawn and Red Alert.
//
// Include this only after the game's RTTIType is declared, and only from a
// standalone build. Remastered does not include it: GlyphX already has a
// queue, and HouseClass must not grow a second one.
//
// Type is that RTTIType (one byte in both games). ID is which unit of that
// kind to build. Each category keeps MAX_PRODUCTION_QUEUE slots in the house:
// the unit being built, plus up to four waiting behind it. All five slots stay
// in the array so an old save still lines up with HouseClass.

struct ProductionQueueEntryStruct
{
    RTTIType Type;
    int ID;
};

enum
{
    MAX_PRODUCTION_QUEUE = 5
};

// Adding, removing, and starting the next item stay in each game's HouseClass.
// Those functions are not the same: Red Alert also queues vessels, and the two
// games check that the house is valid in different ways.

static_assert(sizeof(RTTIType) == 1, "RTTIType must stay one byte so a queued item does not grow");
static_assert(sizeof(ProductionQueueEntryStruct) == 8, "a queued build item must stay 8 bytes");
static_assert(offsetof(ProductionQueueEntryStruct, Type) == 0, "the type must stay the first byte of a queued item");
static_assert(offsetof(ProductionQueueEntryStruct, ID) == 4, "the id must stay at byte 4 of a queued item");
static_assert(MAX_PRODUCTION_QUEUE == 5, "each category keeps five slots");

#endif
