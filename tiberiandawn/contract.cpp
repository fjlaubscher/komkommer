#include "function.h"

#include <stddef.h>

// The numbers below are the layout this tree already compiles. A failing assert
// is a layout change: saved games dump these classes with their padding, and a
// LAN packet is EventClass, so the old bytes no longer load or match a peer.
// After an intentional change, update the numbers here. Do not move a field to
// make an assert match.

#if defined(__GNUC__)
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Winvalid-offsetof"
#endif

static_assert(EventClass::EMPTY == 0, "event id");
static_assert(EventClass::ALLY == 1, "event id");
static_assert(EventClass::MEGAMISSION == 2, "event id");
static_assert(EventClass::IDLE == 3, "event id");
static_assert(EventClass::SCATTER == 4, "event id");
static_assert(EventClass::DESTRUCT == 5, "event id");
static_assert(EventClass::DEPLOY == 6, "event id");
static_assert(EventClass::PLACE == 7, "event id");
static_assert(EventClass::OPTIONS == 8, "event id");
static_assert(EventClass::GAMESPEED == 9, "event id");
static_assert(EventClass::PRODUCE == 10, "event id");
static_assert(EventClass::SUSPEND == 11, "event id");
static_assert(EventClass::ABANDON == 12, "event id");
static_assert(EventClass::PRIMARY == 13, "event id");
static_assert(EventClass::SPECIAL_PLACE == 14, "event id");
static_assert(EventClass::EXIT == 15, "event id");
static_assert(EventClass::ANIMATION == 16, "event id");
static_assert(EventClass::REPAIR == 17, "event id");
static_assert(EventClass::SELL == 18, "event id");
static_assert(EventClass::SPECIAL == 19, "event id");
static_assert(EventClass::FRAMESYNC == 20, "event id");
static_assert(EventClass::MESSAGE == 21, "event id");
static_assert(EventClass::RESPONSE_TIME == 22, "event id");
static_assert(EventClass::FRAMEINFO == 23, "event id");
static_assert(EventClass::ARCHIVE == 24, "event id");
static_assert(EventClass::TIMING == 25, "event id");
static_assert(EventClass::PROCESS_TIME == 26, "event id");
static_assert(EventClass::RALLY == 27, "event id");
static_assert(EventClass::CANCEL_QUEUE == 28, "event id");
static_assert(EventClass::PLACE_LINE == 29, "event id");
static_assert(EventClass::LAST_EVENT == 30, "event id");
static_assert(EventClass::CANCEL_QUEUE == EventClass::RALLY + 1, "CANCEL_QUEUE follows RALLY");
static_assert(EventClass::PLACE_LINE == EventClass::CANCEL_QUEUE + 1, "PLACE_LINE follows CANCEL_QUEUE");
static_assert(EventClass::LAST_EVENT == EventClass::PLACE_LINE + 1, "LAST_EVENT follows PLACE_LINE");

// Same size on LP64, Windows x64, and the 32-bit remaster DLL.
static_assert(sizeof(EventClass) == 22, "EventClass");
static_assert(offsetof(EventClass, Type) == 0, "EventClass::Type");
static_assert(offsetof(EventClass, Data) == 6, "EventClass::Data");
static_assert(sizeof(CommHeaderType) == 8, "CommHeaderType");
static_assert(sizeof(GlobalPacketType) == 48, "GlobalPacketType");
static_assert(sizeof(SerialPacketType) == 88, "SerialPacketType");

#define PIN(type, bytes) static_assert(sizeof(type) == (bytes), #type)
#define AT(type, member, bytes) static_assert(offsetof(type, member) == (bytes), #type "::" #member)

#if defined(REMASTER_BUILD)
static_assert(sizeof(void*) == 4, "remaster DLL is 32-bit");
PIN(AircraftClass, 356);
PIN(AircraftTypeClass, 92);
PIN(AnimClass, 80);
PIN(AnimTypeClass, 92);
PIN(BuildingClass, 348);
PIN(BuildingTypeClass, 212);
PIN(BulletClass, 88);
PIN(BulletTypeClass, 56);
PIN(HouseClass, 3700);
PIN(HouseTypeClass, 60);
PIN(InfantryClass, 340);
PIN(InfantryTypeClass, 368);
PIN(OverlayClass, 40);
PIN(OverlayTypeClass, 60);
PIN(SmudgeClass, 40);
PIN(SmudgeTypeClass, 56);
PIN(TeamClass, 76);
PIN(TeamTypeClass, 228);
PIN(TemplateClass, 40);
PIN(TemplateTypeClass, 52);
PIN(TerrainClass, 48);
PIN(TerrainTypeClass, 64);
PIN(UnitClass, 360);
PIN(UnitTypeClass, 100);
PIN(MouseClass, 2564);
PIN(CellClass, 48);
PIN(FactoryClass, 28);
PIN(BaseClass, 32);
PIN(LayerClass, 24);
PIN(ScenarioClass, 1116);
AT(HouseClass, Credits, 276);
AT(BuildingClass, Factory, 252);
AT(BuildingClass, Rally, 292);
AT(BuildingClass, LastRally, 294);
AT(AircraftClass, Altitude, 336);
AT(UnitClass, ArchiveTarget, 136);
AT(InfantryClass, Fear, 336);
AT(AnimClass, SortTarget, 44);
AT(CellClass, TType, 4);
AT(ScenarioClass, Waypoint, 6);
AT(ScenarioClass, BriefingText, 580);
#elif defined(_WIN64)
static_assert(sizeof(void*) == 8, "Windows standalone is 64-bit");
PIN(AircraftClass, 424);
PIN(AircraftTypeClass, 112);
PIN(AnimClass, 120);
PIN(AnimTypeClass, 104);
PIN(BuildingClass, 408);
PIN(BuildingTypeClass, 256);
PIN(BulletClass, 120);
PIN(BulletTypeClass, 72);
PIN(HouseClass, 7264);
PIN(HouseTypeClass, 72);
PIN(InfantryClass, 408);
PIN(InfantryTypeClass, 392);
PIN(OverlayClass, 64);
PIN(OverlayTypeClass, 72);
PIN(SmudgeClass, 64);
PIN(SmudgeTypeClass, 72);
PIN(TeamClass, 96);
PIN(TeamTypeClass, 256);
PIN(TemplateClass, 64);
PIN(TemplateTypeClass, 72);
PIN(TerrainClass, 80);
PIN(TerrainTypeClass, 88);
PIN(UnitClass, 440);
PIN(UnitTypeClass, 120);
PIN(MouseClass, 3288);
PIN(CellClass, 72);
PIN(FactoryClass, 40);
PIN(BaseClass, 48);
PIN(LayerClass, 32);
PIN(ScenarioClass, 1116);
AT(HouseClass, Credits, 280);
AT(HouseClass, ProductionQueue, 1604);
AT(HouseClass, ProductionQueueCount, 1804);
AT(HouseClass, Radar, 1824);
AT(BuildingClass, Factory, 304);
AT(BuildingClass, Rally, 348);
AT(BuildingClass, LastRally, 350);
AT(AircraftClass, Altitude, 404);
AT(UnitClass, ArchiveTarget, 176);
AT(InfantryClass, Fear, 404);
AT(AnimClass, SortTarget, 72);
AT(CellClass, TType, 4);
AT(ScenarioClass, Waypoint, 6);
AT(ScenarioClass, BriefingText, 580);
#elif !defined(_WIN32)
static_assert(sizeof(void*) == 8, "LP64 standalone is 64-bit");
static_assert(sizeof(long) == 8, "LP64 standalone keeps a 64-bit long");
PIN(AircraftClass, 392);
PIN(AircraftTypeClass, 104);
PIN(AnimClass, 104);
PIN(AnimTypeClass, 104);
PIN(BuildingClass, 376);
PIN(BuildingTypeClass, 240);
PIN(BulletClass, 104);
PIN(BulletTypeClass, 64);
PIN(HouseClass, 7248);
PIN(HouseTypeClass, 72);
PIN(InfantryClass, 384);
PIN(InfantryTypeClass, 376);
PIN(OverlayClass, 56);
PIN(OverlayTypeClass, 72);
PIN(SmudgeClass, 56);
PIN(SmudgeTypeClass, 72);
PIN(TeamClass, 96);
PIN(TeamTypeClass, 256);
PIN(TemplateClass, 56);
PIN(TemplateTypeClass, 72);
PIN(TerrainClass, 64);
PIN(TerrainTypeClass, 80);
PIN(UnitClass, 400);
PIN(UnitTypeClass, 112);
PIN(MouseClass, 3256);
PIN(CellClass, 72);
PIN(FactoryClass, 40);
PIN(BaseClass, 48);
PIN(LayerClass, 32);
PIN(ScenarioClass, 864);
AT(HouseClass, Credits, 276);
AT(HouseClass, ProductionQueue, 1600);
AT(HouseClass, ProductionQueueCount, 1800);
AT(HouseClass, Radar, 1820);
AT(BuildingClass, Factory, 280);
AT(BuildingClass, Rally, 320);
AT(BuildingClass, LastRally, 322);
AT(AircraftClass, Altitude, 372);
AT(UnitClass, ArchiveTarget, 156);
AT(InfantryClass, Fear, 380);
AT(AnimClass, SortTarget, 56);
AT(CellClass, TType, 1);
AT(ScenarioClass, Waypoint, 6);
AT(ScenarioClass, BriefingText, 331);
#endif

#undef PIN
#undef AT

#if defined(__GNUC__)
#pragma GCC diagnostic pop
#endif
