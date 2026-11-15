#include "function.h"

#include <stddef.h>

// The numbers below are the layout this tree already compiles. They are here so a
// later edit that reorders a field, or changes a packet, fails the build. Do not
// move a field to make an assert match.

#if defined(__GNUC__)
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Winvalid-offsetof"
#endif

static_assert(EventClass::EMPTY == 0, "event id");
static_assert(EventClass::ALLY == 1, "event id");
static_assert(EventClass::MEGAMISSION == 2, "event id");
static_assert(EventClass::MEGAMISSION_F == 3, "event id");
static_assert(EventClass::IDLE == 4, "event id");
static_assert(EventClass::SCATTER == 5, "event id");
static_assert(EventClass::DESTRUCT == 6, "event id");
static_assert(EventClass::DEPLOY == 7, "event id");
static_assert(EventClass::PLACE == 8, "event id");
static_assert(EventClass::OPTIONS == 9, "event id");
static_assert(EventClass::GAMESPEED == 10, "event id");
static_assert(EventClass::PRODUCE == 11, "event id");
static_assert(EventClass::SUSPEND == 12, "event id");
static_assert(EventClass::ABANDON == 13, "event id");
static_assert(EventClass::PRIMARY == 14, "event id");
static_assert(EventClass::SPECIAL_PLACE == 15, "event id");
static_assert(EventClass::EXIT == 16, "event id");
static_assert(EventClass::ANIMATION == 17, "event id");
static_assert(EventClass::REPAIR == 18, "event id");
static_assert(EventClass::SELL == 19, "event id");
static_assert(EventClass::SELLCELL == 20, "event id");
static_assert(EventClass::SPECIAL == 21, "event id");
static_assert(EventClass::FRAMESYNC == 22, "event id");
static_assert(EventClass::MESSAGE == 23, "event id");
static_assert(EventClass::RESPONSE_TIME == 24, "event id");
static_assert(EventClass::FRAMEINFO == 25, "event id");
static_assert(EventClass::SAVEGAME == 26, "event id");
static_assert(EventClass::ARCHIVE == 27, "event id");
static_assert(EventClass::ADDPLAYER == 28, "event id");
static_assert(EventClass::TIMING == 29, "event id");
static_assert(EventClass::PROCESS_TIME == 30, "event id");
static_assert(EventClass::PROPOSE_DRAW == 31, "event id");
static_assert(EventClass::RETRACT_DRAW == 32, "event id");
static_assert(EventClass::RALLY == 33, "event id");
static_assert(EventClass::CANCEL_QUEUE == 34, "event id");
static_assert(EventClass::LAST_EVENT == 35, "event id");
static_assert(EventClass::CANCEL_QUEUE == EventClass::RALLY + 1, "CANCEL_QUEUE follows RALLY");
static_assert(EventClass::LAST_EVENT == EventClass::CANCEL_QUEUE + 1, "LAST_EVENT follows CANCEL_QUEUE");

// Same size on LP64, Windows x64, and the 32-bit remaster DLL.
static_assert(sizeof(EventClass) == 20, "EventClass");
static_assert(offsetof(EventClass, Type) == 0, "EventClass::Type");
static_assert(offsetof(EventClass, Data) == 5, "EventClass::Data");
static_assert(sizeof(CommHeaderType) == 8, "CommHeaderType");
static_assert(sizeof(GlobalPacketType) == 141, "GlobalPacketType");
static_assert(sizeof(SerialPacketType) == 150, "SerialPacketType");
static_assert(sizeof(RemoteFileTransferType) == 200, "RemoteFileTransferType");

#define PIN(type, bytes) static_assert(sizeof(type) == (bytes), #type)
#define AT(type, member, bytes) static_assert(offsetof(type, member) == (bytes), #type "::" #member)

#if defined(REMASTER_BUILD)
static_assert(sizeof(void*) == 4, "remaster DLL is 32-bit");
PIN(AircraftClass, 576);
PIN(AircraftTypeClass, 428);
PIN(AnimClass, 128);
PIN(AnimTypeClass, 380);
PIN(BaseClass, 32);
PIN(BuildingClass, 512);
PIN(BuildingTypeClass, 544);
PIN(BulletClass, 100);
PIN(BulletTypeClass, 332);
PIN(CellClass, 96);
PIN(FactoryClass, 56);
PIN(HouseClass, 7908);
PIN(HouseTypeClass, 336);
PIN(InfantryClass, 572);
PIN(InfantryTypeClass, 440);
PIN(LayerClass, 24);
PIN(MouseClass, 9676);
PIN(OverlayClass, 56);
PIN(OverlayTypeClass, 332);
PIN(SmudgeClass, 56);
PIN(SmudgeTypeClass, 332);
PIN(TeamClass, 116);
PIN(TeamTypeClass, 272);
PIN(TemplateClass, 56);
PIN(TemplateTypeClass, 324);
PIN(TerrainClass, 84);
PIN(TerrainTypeClass, 340);
PIN(TriggerClass, 64);
PIN(TriggerTypeClass, 112);
PIN(UnitClass, 620);
PIN(UnitTypeClass, 428);
PIN(VesselClass, 616);
PIN(ScenarioClass, 1984);
PIN(ChronalVortexClass, 4432);
AT(HouseClass, Credits, 520);
AT(BuildingClass, Rally, 456);
AT(BuildingClass, LastRally, 458);
AT(AircraftClass, Height, 16);
AT(UnitClass, ArchiveTarget, 288);
AT(InfantryClass, Fear, 560);
AT(VesselClass, DoorShutCountDown, 580);
AT(AnimClass, SortTarget, 84);
AT(CellClass, TType, 20);
AT(TriggerClass, Event1, 12);
AT(ScenarioClass, Waypoint, 20);
#elif defined(_WIN64)
static_assert(sizeof(void*) == 8, "Windows standalone is 64-bit");
PIN(AircraftClass, 624);
PIN(AircraftTypeClass, 472);
PIN(AnimClass, 136);
PIN(AnimTypeClass, 408);
PIN(BaseClass, 48);
PIN(BuildingClass, 544);
PIN(BuildingTypeClass, 608);
PIN(BulletClass, 120);
PIN(BulletTypeClass, 352);
PIN(CellClass, 112);
PIN(FactoryClass, 72);
PIN(HouseClass, 8128);
PIN(HouseTypeClass, 344);
PIN(InfantryClass, 616);
PIN(InfantryTypeClass, 496);
PIN(LayerClass, 32);
PIN(MouseClass, 9752);
PIN(OverlayClass, 72);
PIN(OverlayTypeClass, 352);
PIN(SmudgeClass, 72);
PIN(SmudgeTypeClass, 352);
PIN(TeamClass, 128);
PIN(TeamTypeClass, 320);
PIN(TemplateClass, 72);
PIN(TemplateTypeClass, 344);
PIN(TerrainClass, 96);
PIN(TerrainTypeClass, 368);
PIN(TriggerClass, 64);
PIN(TriggerTypeClass, 120);
PIN(UnitClass, 664);
PIN(UnitTypeClass, 472);
PIN(VesselClass, 664);
PIN(ScenarioClass, 1984);
PIN(ChronalVortexClass, 4440);
AT(HouseClass, Credits, 520);
AT(HouseClass, ProductionQueue, 2244);
AT(HouseClass, ProductionQueueCount, 2444);
AT(HouseClass, Radar, 2464);
AT(BuildingClass, Rally, 488);
AT(BuildingClass, LastRally, 490);
AT(AircraftClass, Height, 20);
AT(UnitClass, ArchiveTarget, 320);
AT(InfantryClass, Fear, 604);
AT(VesselClass, DoorShutCountDown, 624);
AT(AnimClass, SortTarget, 96);
AT(CellClass, TType, 20);
AT(TriggerClass, Event1, 12);
AT(ScenarioClass, Waypoint, 20);
#elif !defined(_WIN32)
static_assert(sizeof(void*) == 8, "LP64 standalone is 64-bit");
static_assert(sizeof(long) == 8, "LP64 standalone keeps a 64-bit long");
PIN(AircraftClass, 600);
PIN(AircraftTypeClass, 448);
PIN(AnimClass, 128);
PIN(AnimTypeClass, 400);
PIN(BaseClass, 48);
PIN(BuildingClass, 528);
PIN(BuildingTypeClass, 584);
PIN(BulletClass, 104);
PIN(BulletTypeClass, 344);
PIN(CellClass, 112);
PIN(FactoryClass, 72);
PIN(HouseClass, 8128);
PIN(HouseTypeClass, 96);
PIN(InfantryClass, 592);
PIN(InfantryTypeClass, 472);
PIN(LayerClass, 32);
PIN(MouseClass, 9712);
PIN(OverlayClass, 64);
PIN(OverlayTypeClass, 344);
PIN(SmudgeClass, 64);
PIN(SmudgeTypeClass, 344);
PIN(TeamClass, 128);
PIN(TeamTypeClass, 320);
PIN(TemplateClass, 64);
PIN(TemplateTypeClass, 336);
PIN(TerrainClass, 88);
PIN(TerrainTypeClass, 360);
PIN(TriggerClass, 64);
PIN(TriggerTypeClass, 112);
PIN(UnitClass, 640);
PIN(UnitTypeClass, 448);
PIN(VesselClass, 640);
PIN(ScenarioClass, 1732);
PIN(ChronalVortexClass, 4432);
AT(HouseClass, Credits, 520);
AT(HouseClass, ProductionQueue, 2244);
AT(HouseClass, ProductionQueueCount, 2444);
AT(HouseClass, Radar, 2464);
AT(BuildingClass, Rally, 472);
AT(BuildingClass, LastRally, 474);
AT(AircraftClass, Height, 20);
AT(UnitClass, ArchiveTarget, 308);
AT(InfantryClass, Fear, 581);
AT(VesselClass, DoorShutCountDown, 600);
AT(AnimClass, SortTarget, 88);
AT(CellClass, TType, 16);
AT(TriggerClass, Event1, 12);
AT(ScenarioClass, Waypoint, 20);
#endif

#undef PIN
#undef AT

#if defined(__GNUC__)
#pragma GCC diagnostic pop
#endif
