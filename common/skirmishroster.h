// Lobby-only data; never embed this roster in a memory-dumped engine class.
#ifndef COMMON_SKIRMISH_ROSTER_H
#define COMMON_SKIRMISH_ROSTER_H

inline int Skirmish_Clamp(int value, int low, int high)
{
    return value < low ? low : (value > high ? high : value);
}

struct SkirmishSlot
{
    int Faction; // -1 = random; other ids are interpreted by the game.
    int Color;
    int Team; // Zero means free-for-all, not a shared team.
};

template<int PlayerLimit>
struct SkirmishRoster
{
    int Count;
    SkirmishSlot Slots[PlayerLimit];

    SkirmishRoster()
        : Count(2)
    {
        for (int i = 0; i < PlayerLimit; ++i) {
            Slots[i] = SkirmishSlot{i == 0 ? 0 : -1, i, 0};
        }
    }

    void Normalize_Roster(int faction_count)
    {
        Count = Skirmish_Clamp(Count, 1, PlayerLimit);
        bool used[PlayerLimit] = {};
        for (int i = 0; i < PlayerLimit; ++i) {
            Slots[i].Faction = Skirmish_Clamp(Slots[i].Faction, i == 0 ? 0 : -1, faction_count - 1);
            Slots[i].Team = Skirmish_Clamp(Slots[i].Team, 0, PlayerLimit);
            int color = Skirmish_Clamp(Slots[i].Color, 0, PlayerLimit - 1);
            if (used[color]) {
                for (color = 0; used[color]; ++color) {
                }
            }
            Slots[i].Color = color;
            used[color] = true;
        }
    }

    void Select_Color(int row, int color)
    {
        if (row < 0 || row >= Count || color < 0 || color >= PlayerLimit) {
            return;
        }
        for (int i = 0; i < PlayerLimit; ++i) {
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
        SkirmishSlot removed = Slots[row];
        for (int i = row; i + 1 < Count; ++i) {
            Slots[i] = Slots[i + 1];
        }
        Slots[--Count] = SkirmishSlot{-1, removed.Color, 0};
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
};
#endif
