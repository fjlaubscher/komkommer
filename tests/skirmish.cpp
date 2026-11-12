#include <algorithm>
#include <cstdio>
#include <cstdlib>
#include <vector>
#include "tiberiandawn/skirmishsettings.h"

static void Check(bool condition, char const* message)
{
    if (!condition) {
        std::fprintf(stderr, "%s\n", message);
        std::exit(1);
    }
}

int main()
{
    using namespace TDSkirmish;
    Settings settings;
    Check(settings.Count == 2 && settings.Has_Opponent(), "default match has an opponent");
    Check(!settings.Allied(0, 1), "None is free-for-all, not a shared team");
    settings.Slots[0].Team = settings.Slots[1].Team = 2;
    Check(settings.Allied(0, 1) && !settings.Has_Opponent(), "same-team-only match cannot start");
    settings.Count = 3;
    Check(settings.Has_Opponent(), "an unteamed AI is an enemy of a numbered team");
    settings.Select_Color(0, 1);
    Check(settings.Slots[0].Color == 1 && settings.Slots[1].Color == 0, "explicit selection swaps occupied colours");
    Settings selection;
    selection.Count = PLAYER_LIMIT;
    selection.Select_Color(0, 4);
    for (int i = 0; i < PLAYER_LIMIT; ++i) {
        Check(selection.Slots[i].Color == (i == 0 ? 4 : (i == 4 ? 0 : i)),
              "direct colour choice changes only the two selected players, not intermediate colours");
    }
    selection.Select_Color(0, 4);
    Check(selection.Slots[0].Color == 4 && selection.Slots[4].Color == 0, "choosing current colour is a no-op");
    selection.Select_Color(0, -1);
    selection.Select_Color(0, PLAYER_LIMIT);
    Check(selection.Slots[0].Color == 4, "invalid selections do not change colours");
    selection = Settings();
    selection.Select_Color(0, 5);
    Check(selection.Slots[0].Color == 5 && selection.Slots[1].Color == 1 && selection.Slots[5].Color == 0,
          "selecting an unused colour leaves all other active players alone and reserves colours for future AI");
    settings.Remove(1);
    Check(settings.Count == 2 && settings.Slots[1].Color == 2 && settings.Slots[2].Color == 0,
          "removing an AI compacts the roster and preserves unique colours");
    settings.Remove(0);
    Check(settings.Count == 2, "human cannot be removed");

    settings.Name = "Commander";
    settings.Scenario = 37;
    settings.Credits = 4321;
    settings.Units = 3;
    settings.TechLevel = 5;
    settings.Difficulty = 4;
    settings.Bases = false;
    settings.Tiberium = false;
    settings.Crates = true;
    settings.CaptureTheFlag = true;
    settings.Slots[1].Faction = 1;
    settings.Slots[1].Team = 4;
    INIClass ini;
    ini.Put_String("Options", "Untouched", "yes");
    settings.Save(ini);
    Settings loaded;
    loaded.Load(ini);
    loaded.Normalize();
    Check(loaded.Name == settings.Name && loaded.Scenario == 37 && loaded.Count == 2,
          "name, map identity, and roster count round-trip");
    Check(loaded.Credits == 4321 && loaded.Units == 3 && loaded.TechLevel == 5 && loaded.Difficulty == 4,
          "numeric settings round-trip");
    Check(!loaded.Bases && !loaded.Tiberium && loaded.Crates && loaded.CaptureTheFlag, "rules round-trip");
    for (int i = 0; i < PLAYER_LIMIT; ++i) {
        Check(loaded.Slots[i].Faction == settings.Slots[i].Faction && loaded.Slots[i].Color == settings.Slots[i].Color
              && loaded.Slots[i].Team == settings.Slots[i].Team, "all roster fields round-trip, including inactive slots");
    }
    Check(ini.Get_String("Options", "Untouched", std::string()) == "yes", "saving preserves other config sections");

    ini.Put_Int("Skirmish", "Players", 99);
    ini.Put_Int("Skirmish", "Credits", -100);
    ini.Put_Int("Skirmish", "Units", 999);
    ini.Put_Int("Skirmish", "TechLevel", -1);
    ini.Put_Int("Skirmish", "Difficulty", 99);
    for (int i = 0; i < PLAYER_LIMIT; ++i) {
        char key[32];
        std::sprintf(key, "Player%dColor", i);
        ini.Put_Int("Skirmish", key, 99);
        std::sprintf(key, "Player%dFaction", i);
        ini.Put_Int("Skirmish", key, -99);
        std::sprintf(key, "Player%dTeam", i);
        ini.Put_Int("Skirmish", key, -99);
    }
    loaded.Load(ini);
    loaded.Normalize();
    Check(loaded.Count == 6 && loaded.Credits == 0 && loaded.Units == 50 && loaded.TechLevel == 1
          && loaded.Difficulty == 4, "malformed numeric settings are clamped");
    bool colors[PLAYER_LIMIT] = {};
    for (int i = 0; i < PLAYER_LIMIT; ++i) {
        Check(!colors[loaded.Slots[i].Color], "duplicate/out-of-range colours are repaired");
        colors[loaded.Slots[i].Color] = true;
        Check(loaded.Slots[i].Team == 0 && loaded.Slots[i].Faction == (i == 0 ? 0 : -1), "invalid slots repaired");
    }
    loaded.Bases = true;
    loaded.Normalize();
    Check(loaded.Units == 12, "base toggle clamps starting-unit range");

    INIClass map_ini;
    map_ini.Put_Int("Map", "Width", 32);
    map_ini.Put_Int("Map", "Height", 24);
    map_ini.Put_Int("Waypoints", "0", 65);
    map_ini.Put_Int("Waypoints", "4", 130);
    map_ini.Put_Int("Waypoints", "5", 130); // duplicate
    map_ini.Put_Int("Waypoints", "6", -1);
    map_ini.Put_Int("Waypoints", "7", 4096); // outside grid
    map_ini.Put_Int("Waypoints", "8", 63); // outside playable bounds
    map_ini.Put_Int("Waypoints", "26", 195); // not used by Create_Units
    MapInfo map;
    map.Read(map_ini, 128);
    Check(map.Width == 32 && map.Height == 24 && map.Players == 2,
          "map metadata counts distinct, valid, possibly non-contiguous starts");
    map_ini.Put_Int("Map", "Version", 1);
    map_ini.Put_Int("Waypoints", "0", 129);
    map_ini.Put_Int("Waypoints", "4", 258);
    map_ini.Put_Int("Waypoints", "5", 258);
    map.Read(map_ini, 128);
    Check(map.Players == 2, "megamap waypoints use their own stride");
    map_ini.Put_Int("Map", "Width", 999999);
    map.Read(map_ini, 128);
    Check(map.Players == 0 && map.Width == 0, "invalid map dimensions cannot start a game");

    std::vector<MapInfo> maps(4);
    for (int i = 0; i < 4; ++i) {
        maps[i].Scenario = 90 - i;
        maps[i].OriginalIndex = i;
        maps[i].Width = maps[i].Height = 32;
        maps[i].Name = i == 0 ? "ZETA" : "ALPHA";
    }
    maps[2].Width = maps[2].Height = 16;
    maps[3].Width = maps[3].Height = 0;
    std::sort(maps.begin(), maps.end());
    Check(maps[0].Scenario == 88 && maps[1].Scenario == 89 && maps[2].Scenario == 90 && maps[3].Scenario == 87,
          "maps sort by playable area then name, invalid maps last, retaining identity");
    return 0;
}
