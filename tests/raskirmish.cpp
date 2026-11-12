#include <algorithm>
#include <cstdio>
#include <cstdlib>
#include <vector>
#include "redalert/skirmishsettings.h"
#include "redalert/skirmishlayout.h"

static void Check(bool condition, char const* message)
{
    if (!condition) {
        std::fprintf(stderr, "%s\n", message);
        std::exit(1);
    }
}

static bool Contains(RASkirmish::Layout::Rect outer, RASkirmish::Layout::Rect inner)
{
    return inner.X >= outer.X && inner.Y >= outer.Y
           && inner.X + inner.Width <= outer.X + outer.Width
           && inner.Y + inner.Height <= outer.Y + outer.Height;
}

static bool Overlaps(RASkirmish::Layout::Rect a, RASkirmish::Layout::Rect b)
{
    return a.X < b.X + b.Width && b.X < a.X + a.Width
           && a.Y < b.Y + b.Height && b.Y < a.Y + a.Height;
}

static RASkirmish::Layout::Rect Scale(RASkirmish::Layout::Rect rect, int factor)
{
    return RASkirmish::Layout::Rect{rect.X * factor, rect.Y * factor,
                                   rect.Width * factor, rect.Height * factor};
}

static void Check_Layout()
{
    using namespace RASkirmish;
    Layout::Rect const regions[] = {Layout::Name, Layout::MapList, Layout::MapInfo, Layout::Roster,
                                    Layout::AddAI, Layout::Credits, Layout::Units, Layout::Tech,
                                    Layout::Difficulty, Layout::Rules, Layout::ExtraRules,
                                    Layout::Start, Layout::Defaults, Layout::Back};
    for (int factor = 1; factor <= 2; ++factor) {
        Layout::Rect const screen = {0, 0, 320 * factor, 200 * factor};
        Layout::Rect const interior = Scale(Layout::Interior, factor);
        Check(Contains(screen, Scale(Layout::Dialog, factor)), "RA dialog stays inside the screen");
        for (unsigned i = 0; i < sizeof(regions) / sizeof(regions[0]); ++i) {
            Check(Contains(interior, Scale(regions[i], factor)), "RA lobby controls stay clear of the native metal frame");
            for (unsigned j = i + 1; j < sizeof(regions) / sizeof(regions[0]); ++j) {
                Check(!Overlaps(Scale(regions[i], factor), Scale(regions[j], factor)),
                      "RA map list, eight-player roster, compact settings and footer do not overlap");
            }
        }
        for (int row = 0; row < PLAYER_LIMIT; ++row) {
            for (int col = 0; col < 4; ++col) {
                Layout::Rect control = {Layout::ColumnX[col], Layout::Roster.Y + row * Layout::RowPitch,
                                         Layout::ColumnWidth[col], Layout::RowPitch};
                Check(Contains(Scale(Layout::Roster, factor), Scale(control, factor)),
                      "all eight player rows, including remove buttons, fit the roster bounds");
                if (col < 3) {
                    int choices = col == 0 ? COUNTRY_COUNT + (row != 0) : (col == 1 ? PLAYER_LIMIT : PLAYER_LIMIT + 1);
                    Layout::Rect popup = {control.X, control.Y + Layout::RowPitch,
                                           control.Width, choices * 8 + 2};
                    Check(Contains(interior, Scale(popup, factor)),
                          "roster dropdowns fit inside the frame even with an eight-pixel logical line height");
                }
            }
        }
    }
    Check(Layout::MapList.Height >= 120, "RA map list keeps the extra vertical space reclaimed from match settings");
    Check(Layout::Back.X < Layout::Defaults.X && Layout::Defaults.X < Layout::Start.X,
          "RA footer orders Back, Restore defaults, then Start from left to right");
}

int main()
{
    using namespace RASkirmish;
    Check_Layout();
    Settings settings;
    Check(settings.Count == 2 && settings.Has_Opponent(), "RA defaults include an opponent");
    settings.Count = PLAYER_LIMIT;
    settings.Select_Color(0, 7);
    for (int i = 0; i < PLAYER_LIMIT; ++i) {
        Check(settings.Slots[i].Color == (i == 0 ? 7 : (i == 7 ? 0 : i)), "RA eighth colour swaps just two slots");
        settings.Slots[i].Team = 8;
    }
    Check(!settings.Has_Opponent() && settings.Allied(0, 7), "RA team 8 forms mutual alliances");
    settings.Slots[7].Team = 0;
    Check(settings.Has_Opponent() && !settings.Allied(0, 7), "None is not allied with a numbered team");
    settings.Slots[0].Team = 0;
    Check(!settings.Allied(0, 7), "two unteamed RA players are enemies");
    settings.Slots[7].Faction = 4;
    settings.Remove(3);
    Check(settings.Count == 7 && settings.Slots[6].Faction == 4 && settings.Slots[6].Color == 0,
          "removing a middle AI preserves the eighth player's country and colour");

    settings.Name = "Commander";
    settings.Scenario = "custom.mpr";
    settings.Credits = 17000;
    settings.TechLevel = 10;
    settings.Units = 40;
    settings.Difficulty = 3;
    settings.Bases = false;
    settings.Ore = false;
    settings.Crates = false;
    settings.Shadow = false;
    settings.CaptureTheFlag = true;
    settings.Normalize(20000);
    INIClass ini;
    ini.Put_String("Options", "Untouched", "yes");
    settings.Save(ini);
    Settings loaded;
    loaded.Load(ini);
    loaded.Normalize(20000);
    Check(loaded.Name == settings.Name && loaded.Scenario == "custom.mpr" && loaded.Count == 7,
          "RA stores map filenames rather than sorted/filtered indices");
    Check(loaded.Credits == 17000 && loaded.Units == 40 && loaded.TechLevel == 10 && loaded.Difficulty == 3,
          "RA numeric settings respect rules-defined money limits and round-trip");
    Check(!loaded.Bases && !loaded.Ore && !loaded.Crates && !loaded.Shadow && loaded.CaptureTheFlag,
          "all five RA rules persist, including shadow regrowth");
    for (int i = 0; i < PLAYER_LIMIT; ++i) {
        Check(loaded.Slots[i].Faction == settings.Slots[i].Faction && loaded.Slots[i].Color == settings.Slots[i].Color
              && loaded.Slots[i].Team == settings.Slots[i].Team, "all eight RA roster slots round-trip");
    }
    Check(ini.Get_String("Options", "Untouched", std::string()) == "yes", "other RA config sections survive");

    loaded.Count = 99;
    loaded.Credits = 99999;
    loaded.TechLevel = 99;
    loaded.Units = -1;
    loaded.Difficulty = -1;
    for (int i = 0; i < PLAYER_LIMIT; ++i) {
        loaded.Slots[i] = Slot{99, 99, 99};
    }
    loaded.Normalize(15000, 6);
    Check(loaded.Count == 6 && loaded.Credits == 15000 && loaded.TechLevel == 10 && loaded.Units == 1
          && loaded.Difficulty == 0, "malformed RA preferences and custom player limits are clamped");
    bool colors[PLAYER_LIMIT] = {};
    for (int i = 0; i < PLAYER_LIMIT; ++i) {
        Check(!colors[loaded.Slots[i].Color], "RA colours remain unique, including inactive slots");
        colors[loaded.Slots[i].Color] = true;
        Check(loaded.Slots[i].Faction == 4 && loaded.Slots[i].Team == 8, "RA country/team limits are engine-specific");
    }
    loaded.Bases = true;
    loaded.Units = 50;
    loaded.Normalize();
    Check(loaded.Units == 12, "base games clamp the RA starting-unit range");

    INIClass map_ini;
    map_ini.Put_Int("Map", "Width", 32);
    map_ini.Put_Int("Map", "Height", 24);
    map_ini.Put_Int("Waypoints", "0", 129);
    map_ini.Put_Int("Waypoints", "7", 258);
    map_ini.Put_Int("Waypoints", "1", 129);
    map_ini.Put_Int("Waypoints", "2", -1);
    map_ini.Put_Int("Waypoints", "3", 0);
    map_ini.Put_Int("Waypoints", "4", 16384);
    map_ini.Put_Int("Waypoints", "8", 387);
    MapInfo map;
    map.Read(map_ini);
    Check(map.Width == 32 && map.Height == 24 && map.Players == 2,
          "RA uses 128-cell stride and only distinct playable start waypoints 0..7");
    for (int i = 0; i < PLAYER_LIMIT; ++i) {
        char key[8];
        std::sprintf(key, "%d", i);
        map_ini.Put_Int("Waypoints", key, (i + 1) * 128 + 1);
    }
    map.Read(map_ini);
    Check(map.Players == 8, "RA supports eight starting locations");
    map_ini.Put_Int("Map", "Width", 999999);
    map.Read(map_ini);
    Check(map.Width == 0 && map.Players == 0, "invalid RA dimensions cannot start a match");

    std::vector<MapInfo> maps(3);
    for (int i = 0; i < 3; ++i) {
        maps[i].Width = maps[i].Height = i == 0 ? 16 : 32;
        maps[i].Scenario = i == 0 ? "small.ini" : (i == 1 ? "zeta.mpr" : "alpha.ini");
        maps[i].Name = i == 0 ? "SMALL" : (i == 1 ? "ZETA" : "ALPHA");
        maps[i].OriginalIndex = 10 + i;
    }
    std::sort(maps.begin(), maps.end());
    Check(maps[0].Scenario == "small.ini" && maps[1].Scenario == "alpha.ini" && maps[1].OriginalIndex == 12,
          "RA sorting preserves filenames and original session indices");
    return 0;
}
