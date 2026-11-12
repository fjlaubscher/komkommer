// Standalone RA lobby preferences, separate from SessionClass/HouseClass saves.
#ifndef RA_SKIRMISH_SETTINGS_H
#define RA_SKIRMISH_SETTINGS_H

#include <cstdio>
#include <string>
#include "ini.h"
#include "common/skirmishroster.h"

namespace RASkirmish
{
enum { PLAYER_LIMIT = 8, COUNTRY_COUNT = 5 };
using Slot = SkirmishSlot;

struct Settings : SkirmishRoster<PLAYER_LIMIT>
{
    std::string Name;
    std::string Scenario; // Filename, not a volatile Session.Scenarios index.
    int Credits;
    int Units;
    int TechLevel;
    int Difficulty;
    bool Bases;
    bool Ore;
    bool Crates;
    bool Shadow;
    bool CaptureTheFlag;

    Settings()
        : Name("Noname")
        , Credits(3000)
        , Units(6)
        , TechLevel(10)
        , Difficulty(2)
        , Bases(true)
        , Ore(true)
        , Crates(true)
        , Shadow(true)
        , CaptureTheFlag(false)
    {
    }

    void Normalize(int max_credits = 10000, int max_players = PLAYER_LIMIT)
    {
        Normalize_Roster(COUNTRY_COUNT);
        Count = Skirmish_Clamp(Count, 1, Skirmish_Clamp(max_players, 1, PLAYER_LIMIT));
        Credits = Skirmish_Clamp(Credits, 0, max_credits > 0 ? max_credits : 0);
        Units = Skirmish_Clamp(Units, Bases ? 0 : 1, Bases ? 12 : 50);
        TechLevel = Skirmish_Clamp(TechLevel, 1, 10);
        Difficulty = Skirmish_Clamp(Difficulty, 0, 4);
    }

    void Load(INIClass const& ini)
    {
        Name = ini.Get_String("Skirmish", "Name", Name);
        Scenario = ini.Get_String("Skirmish", "Map", Scenario);
        Count = ini.Get_Int("Skirmish", "Players", Count);
        Credits = ini.Get_Int("Skirmish", "Credits", Credits);
        Units = ini.Get_Int("Skirmish", "Units", Units);
        TechLevel = ini.Get_Int("Skirmish", "TechLevel", TechLevel);
        Difficulty = ini.Get_Int("Skirmish", "Difficulty", Difficulty);
        Bases = ini.Get_Bool("Skirmish", "Bases", Bases);
        Ore = ini.Get_Bool("Skirmish", "Ore", Ore);
        Crates = ini.Get_Bool("Skirmish", "Crates", Crates);
        Shadow = ini.Get_Bool("Skirmish", "ShadowRegrows", Shadow);
        CaptureTheFlag = ini.Get_Bool("Skirmish", "CaptureTheFlag", CaptureTheFlag);
        for (int i = 0; i < PLAYER_LIMIT; ++i) {
            char key[32];
            std::sprintf(key, "Player%dFaction", i);
            Slots[i].Faction = ini.Get_Int("Skirmish", key, Slots[i].Faction);
            std::sprintf(key, "Player%dColor", i);
            Slots[i].Color = ini.Get_Int("Skirmish", key, Slots[i].Color);
            std::sprintf(key, "Player%dTeam", i);
            Slots[i].Team = ini.Get_Int("Skirmish", key, Slots[i].Team);
        }
    }

    void Save(INIClass& ini) const
    {
        ini.Put_String("Skirmish", "Name", Name);
        ini.Put_String("Skirmish", "Map", Scenario);
        ini.Put_Int("Skirmish", "Players", Count);
        ini.Put_Int("Skirmish", "Credits", Credits);
        ini.Put_Int("Skirmish", "Units", Units);
        ini.Put_Int("Skirmish", "TechLevel", TechLevel);
        ini.Put_Int("Skirmish", "Difficulty", Difficulty);
        ini.Put_Bool("Skirmish", "Bases", Bases);
        ini.Put_Bool("Skirmish", "Ore", Ore);
        ini.Put_Bool("Skirmish", "Crates", Crates);
        ini.Put_Bool("Skirmish", "ShadowRegrows", Shadow);
        ini.Put_Bool("Skirmish", "CaptureTheFlag", CaptureTheFlag);
        for (int i = 0; i < PLAYER_LIMIT; ++i) {
            char key[32];
            std::sprintf(key, "Player%dFaction", i);
            ini.Put_Int("Skirmish", key, Slots[i].Faction);
            std::sprintf(key, "Player%dColor", i);
            ini.Put_Int("Skirmish", key, Slots[i].Color);
            std::sprintf(key, "Player%dTeam", i);
            ini.Put_Int("Skirmish", key, Slots[i].Team);
        }
    }
};

struct MapInfo
{
    std::string Scenario;
    std::string Name;
    std::string Label;
    int OriginalIndex;
    int Width;
    int Height;
    int Players;

    void Read(INIClass const& ini)
    {
        // RA always uses its 128-cell stride; it has no TD Map.Version conversion.
        int const stride = 128;
        Width = ini.Get_Int("Map", "Width", 126);
        Height = ini.Get_Int("Map", "Height", 126);
        int x = ini.Get_Int("Map", "X", 1);
        int y = ini.Get_Int("Map", "Y", 1);
        Players = 0;
        if (Width < 1 || Height < 1 || Width >= stride || Height >= stride || x < 0 || y < 0
            || x > stride - Width || y > stride - Height) {
            Width = Height = 0;
            return;
        }
        int cells[PLAYER_LIMIT];
        for (int i = 0; i < PLAYER_LIMIT; ++i) {
            char key[12];
            std::sprintf(key, "%d", i);
            int cell = ini.Get_Int("Waypoints", key, -1);
            if (cell < 0 || cell >= stride * stride || cell % stride < x || cell % stride >= x + Width
                || cell / stride < y || cell / stride >= y + Height) {
                continue;
            }
            bool duplicate = false;
            for (int j = 0; j < Players; ++j) {
                duplicate |= cells[j] == cell;
            }
            if (!duplicate) {
                cells[Players++] = cell;
            }
        }
    }

    bool operator<(MapInfo const& other) const
    {
        int area = Width > 0 ? Width * Height : 0x7fffffff;
        int other_area = other.Width > 0 ? other.Width * other.Height : 0x7fffffff;
        if (area != other_area) {
            return area < other_area;
        }
        if (Name != other.Name) {
            return Name < other.Name;
        }
        return Scenario < other.Scenario;
    }
};
} // namespace RASkirmish
#endif
