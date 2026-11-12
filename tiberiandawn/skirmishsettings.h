// Standalone TD lobby data. This is not part of a saved engine object's layout.
#ifndef TD_SKIRMISH_SETTINGS_H
#define TD_SKIRMISH_SETTINGS_H

#include <cstdio>
#include <string>
#include "ini.h"

namespace TDSkirmish
{

enum { PLAYER_LIMIT = 6 };

inline int Clamp(int value, int low, int high)
{
    return value < low ? low : (value > high ? high : value);
}

struct Slot
{
    int Faction; // -1 = random, 0 = GDI, 1 = Nod
    int Color;
    int Team; // 0 = no team; these players are NOT allied with one another
};

struct Settings
{
    std::string Name;
    int Scenario;
    int Count;
    int Credits;
    int Units;
    int TechLevel;
    int Difficulty; // global match handicap, 0..4
    bool Bases;
    bool Tiberium;
    bool Crates;
    bool CaptureTheFlag;
    Slot Slots[PLAYER_LIMIT];

    Settings()
        : Name("Noname")
        , Scenario(-1)
        , Count(2)
        , Credits(10000)
        , Units(6)
        , TechLevel(7)
        , Difficulty(2)
        , Bases(true)
        , Tiberium(true)
        , Crates(false)
        , CaptureTheFlag(false)
    {
        for (int i = 0; i < PLAYER_LIMIT; ++i) {
            Slots[i] = Slot{i == 0 ? 0 : -1, i, 0};
        }
    }

    void Normalize(int max_tech = 7)
    {
        Count = Clamp(Count, 1, PLAYER_LIMIT);
        Credits = Clamp(Credits, 0, 10000);
        Units = Clamp(Units, Bases ? 0 : 1, Bases ? 12 : 50);
        TechLevel = Clamp(TechLevel, 1, max_tech);
        Difficulty = Clamp(Difficulty, 0, 4);
        bool used[PLAYER_LIMIT] = {};
        for (int i = 0; i < PLAYER_LIMIT; ++i) {
            Slots[i].Faction = Clamp(Slots[i].Faction, i == 0 ? 0 : -1, 1);
            Slots[i].Team = Clamp(Slots[i].Team, 0, PLAYER_LIMIT);
            int color = Clamp(Slots[i].Color, 0, PLAYER_LIMIT - 1);
            if (used[color]) {
                for (color = 0; used[color]; ++color) {
                }
            }
            Slots[i].Color = color;
            used[color] = true;
        }
    }

    // Only a deliberate selection swaps colours, including with an inactive slot.
    void Select_Color(int row, int color)
    {
        if (row < 0 || row >= Count || color < 0 || color >= PLAYER_LIMIT) {
            return;
        }
        for (int i = 0; i < PLAYER_LIMIT; ++i) {
            if (i != row && Slots[i].Color == color) {
                Slots[i].Color = Slots[row].Color;
                break;
            }
        }
        Slots[row].Color = color;
    }

    void Remove(int row)
    {
        if (row <= 0 || row >= Count) {
            return;
        }
        Slot removed = Slots[row];
        for (int i = row; i + 1 < Count; ++i) {
            Slots[i] = Slots[i + 1];
        }
        Slots[--Count] = Slot{-1, removed.Color, 0};
    }

    bool Allied(int a, int b) const
    {
        return a == b || (Slots[a].Team != 0 && Slots[a].Team == Slots[b].Team);
    }

    bool Has_Opponent() const
    {
        for (int i = 1; i < Count; ++i) {
            if (!Allied(0, i)) {
                return true;
            }
        }
        return false;
    }

    void Load(INIClass const& ini)
    {
        Name = ini.Get_String("Skirmish", "Name", Name);
        Scenario = ini.Get_Int("Skirmish", "Map", Scenario);
        Count = ini.Get_Int("Skirmish", "Players", Count);
        Credits = ini.Get_Int("Skirmish", "Credits", Credits);
        Units = ini.Get_Int("Skirmish", "Units", Units);
        TechLevel = ini.Get_Int("Skirmish", "TechLevel", TechLevel);
        Difficulty = ini.Get_Int("Skirmish", "Difficulty", Difficulty);
        Bases = ini.Get_Bool("Skirmish", "Bases", Bases);
        Tiberium = ini.Get_Bool("Skirmish", "Tiberium", Tiberium);
        Crates = ini.Get_Bool("Skirmish", "Crates", Crates);
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
        ini.Put_Int("Skirmish", "Map", Scenario);
        ini.Put_Int("Skirmish", "Players", Count);
        ini.Put_Int("Skirmish", "Credits", Credits);
        ini.Put_Int("Skirmish", "Units", Units);
        ini.Put_Int("Skirmish", "TechLevel", TechLevel);
        ini.Put_Int("Skirmish", "Difficulty", Difficulty);
        ini.Put_Bool("Skirmish", "Bases", Bases);
        ini.Put_Bool("Skirmish", "Tiberium", Tiberium);
        ini.Put_Bool("Skirmish", "Crates", Crates);
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
    int Scenario;
    int OriginalIndex;
    int Width;
    int Height;
    int Players;
    std::string Name;
    std::string Label;

    void Read(INIClass const& ini, int grid_width)
    {
        Width = ini.Get_Int("Map", "Width", grid_width - 2);
        Height = ini.Get_Int("Map", "Height", grid_width - 2);
        int x = ini.Get_Int("Map", "X", 1);
        int y = ini.Get_Int("Map", "Y", 1);
        int stride = ini.Get_Int("Map", "Version", 0) == 0 ? 64 : grid_width;
        Players = 0;
        if (Width < 1 || Height < 1 || Width >= stride || Height >= stride || x < 0 || y < 0
            || x > stride - Width || y > stride - Height) {
            Width = Height = 0;
            return;
        }
        int cells[26];
        int count = 0;
        // Create_Units uses waypoints 0..25, including non-contiguous entries.
        for (int i = 0; i < 26; ++i) {
            char key[12];
            std::sprintf(key, "%d", i);
            int cell = ini.Get_Int("Waypoints", key, -1);
            if (cell < 0 || cell >= stride * stride || cell % stride < x || cell % stride >= x + Width
                || cell / stride < y || cell / stride >= y + Height) {
                continue;
            }
            bool duplicate = false;
            for (int j = 0; j < count; ++j) {
                duplicate |= cells[j] == cell;
            }
            if (!duplicate) {
                cells[count++] = cell;
            }
        }
        Players = Clamp(count, 0, PLAYER_LIMIT);
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

} // namespace TDSkirmish
#endif
