#ifndef REMASTER_BUILD
#include <algorithm>
#include <memory>
#include <vector>
#include <cstdlib>
#include <ctime>
#include <cctype>
#include "function.h"
#include "drop.h"
#include "framelimit.h"
#include "paths.h"
#include "skirmish.h"
#include "skirmishsettings.h"
#include "common/skirmishdrop.h"

namespace
{
using namespace TDSkirmish;
static_assert(PLAYER_LIMIT == REMAP_COUNT, "The TD roster requires one unique colour per player");
Settings Match;
bool MatchActive = false;

void Set_Difficulty(int difficulty)
{
    static DiffType const human[] = {DIFF_EASY, DIFF_NORMAL, DIFF_NORMAL, DIFF_NORMAL, DIFF_HARD};
    static DiffType const computer[] = {DIFF_HARD, DIFF_HARD, DIFF_NORMAL, DIFF_EASY, DIFF_EASY};
    Scen.Difficulty = human[difficulty];
    Scen.CDifficulty = computer[difficulty];
}

std::string Config_Path()
{
    return PathsClass::Concatenate_Paths(Paths.User_Path(), CONFIG_FILE_NAME);
}

bool Save_Settings(Settings const& settings)
{
    INIClass ini;
    RawFileClass file(Config_Path().c_str());
    if (file.Is_Available()) {
        ini.Load(file);
    }
    settings.Save(ini);
    RawFileClass out(Config_Path().c_str());
    return ini.Save(out) != 0;
}

using RosterDropListClass = SkirmishDropListClass<DropListClass>;

enum Buttons
{
    NAME = 100,
    MAPS,
    ADD_AI,
    CREDITS,
    UNITS,
    TECH,
    RULES,
    DIFFICULTY,
    DEFAULTS,
    START,
    BACK,
    SLOT_FIRST = 200 // four controls per row: faction, colour, team, remove
};
}

int Skirmish_Dialog()
{
    using namespace TDSkirmish;
    MatchActive = false;
    int const factor = SeenBuff.Get_Width() == 320 ? 1 : 2;
    TextPrintType const text = TPF_6PT_GRAD | TPF_USE_GRAD_PAL | TPF_NOSHADOW;
    TextPrintType const button_text = text | TPF_CENTER;

    Settings settings;
    settings.Name = MPlayerName;
    settings.Slots[0].Faction = MPlayerHouse - HOUSE_GOOD;
    settings.Slots[0].Color = MPlayerPrefColor;
    INIClass config;
    RawFileClass config_file(Config_Path().c_str());
    if (config_file.Is_Available() && config.Load(config_file)) {
        settings.Load(config);
    }
    settings.Normalize(MPLAYER_BUILD_LEVEL_MAX);
    if (!Rule.IsFineDifficulty) {
        settings.Difficulty = (settings.Difficulty / 2) * 2;
    }

    std::vector<MapInfo> maps;
    for (int i = 0; i < MPlayerFilenum.Count(); ++i) {
        char root[32];
        char filename[40];
        Set_Scenario_Name(root, MPlayerFilenum[i], SCEN_PLAYER_MPLAYER, SCEN_DIR_EAST, SCEN_VAR_A);
        std::sprintf(filename, "%s.INI", root);
        CCFileClass file(filename); // Includes maps in MIX archives, not just loose files.
        INIClass ini;
        if (!ini.Load(file)) {
            continue;
        }
        MapInfo map;
        map.Scenario = MPlayerFilenum[i];
        map.OriginalIndex = i;
        map.Name = MPlayerScenarios[i];
        // Use a case-independent name key without modifying the shared MP descriptions.
        std::transform(map.Name.begin(), map.Name.end(), map.Name.begin(), [](unsigned char c) {
            return static_cast<char>(std::toupper(c));
        });
        map.Read(ini, MAP_CELL_W);
        maps.push_back(map);
    }
    if (maps.empty()) {
        WWMessageBox().Process("No skirmish maps are available.");
        return false;
    }
    std::sort(maps.begin(), maps.end());

    void const* up = Hires_Retrieve(factor == 1 || InMainLoop ? "BTN-UP.SHP" : "BTN-UP2.SHP");
    void const* down = Hires_Retrieve(factor == 1 || InMainLoop ? "BTN-DN.SHP" : "BTN-DN2.SHP");
    char name[MPLAYER_NAME_MAX];
    std::snprintf(name, sizeof(name), "%s", settings.Name.c_str());
    EditClass name_edit(NAME, name, sizeof(name), text, 52 * factor, 19 * factor, 94 * factor, 9 * factor,
                        EditClass::ALPHANUMERIC);
    GadgetClass* commands = &name_edit;
    ListClass map_list(MAPS, 10 * factor, 42 * factor, 142 * factor, 70 * factor, text, up, down);
    map_list.Add_Tail(*commands);
    // Reserve the suffix before shortening a name; player capacity must never be clipped away.
    Fancy_Text_Print("", 0, 0, 0, 0, text);
    int selected = 0;
    for (int i = 0; i < static_cast<int>(maps.size()); ++i) {
        char suffix[20];
        std::sprintf(suffix, " (%dP)", maps[i].Players);
        std::string label = maps[i].Name;
        int width = 124 * factor - String_Pixel_Width(suffix);
        if (String_Pixel_Width(label.c_str()) > width) {
            while (!label.empty() && String_Pixel_Width((label + "...").c_str()) > width) {
                label.pop_back();
            }
            label += "...";
        }
        maps[i].Label = label + suffix;
        map_list.Add_Item(maps[i].Label.c_str());
        if (maps[i].Scenario == settings.Scenario) {
            selected = i;
        }
    }
    map_list.Set_Selected_Index(selected);

    TextButtonClass add(ADD_AI, "Add AI", button_text, 180 * factor, 104 * factor, 60 * factor, 9 * factor);
    TextButtonClass difficulty(DIFFICULTY, "", button_text, 81 * factor, 166 * factor, 71 * factor, 10 * factor);
    TextButtonClass back(BACK, "Back", button_text, 10 * factor, 183 * factor, 48 * factor, 10 * factor);
    TextButtonClass defaults(DEFAULTS, "Restore defaults", button_text, 111 * factor, 183 * factor, 98 * factor, 10 * factor);
    TextButtonClass start(START, "Start", button_text, 262 * factor, 183 * factor, 48 * factor, 10 * factor);
    GaugeClass credits(CREDITS, 82 * factor, 128 * factor, 42 * factor, 7 * factor);
    GaugeClass units(UNITS, 82 * factor, 140 * factor, 42 * factor, 7 * factor);
    GaugeClass tech(TECH, 82 * factor, 152 * factor, 42 * factor, 7 * factor);
    CheckListClass rules(RULES, 169 * factor, 128 * factor, 139 * factor, 34 * factor, text, up, down);
    int tabs[] = {8};
    rules.Set_Tabs(tabs);
    rules.Set_Read_Only(false);
    rules.Add_Item("Bases");
    rules.Add_Item("Tiberium regrows");
    rules.Add_Item("Crates");
    rules.Add_Item("Capture the flag");
    GadgetClass* controls[] = {&add, &difficulty, &defaults, &start, &back, &credits, &units, &tech, &rules};
    for (GadgetClass* control : controls) {
        control->Add_Tail(*commands);
    }
    char labels[PLAYER_LIMIT][4][24] = {};
    std::unique_ptr<GadgetClass> slots[PLAYER_LIMIT][4];
    RosterDropListClass* dropdowns[PLAYER_LIMIT * 3];
    static char const* const colors[] = {"Gold", "Grey", "Red", "Green", "Orange", "Dk green"};
    static char const* const teams[] = {"None", "1", "2", "3", "4", "5", "6"};
    int const xs[] = {182, 219, 269, 298};
    int const widths[] = {35, 48, 27, 12};
    for (int row = 0; row < PLAYER_LIMIT; ++row) {
        for (int col = 0; col < 4; ++col) {
            if (col < 3) {
                int item_count = col == 0 ? (row == 0 ? 2 : 3) : (col == 1 ? PLAYER_LIMIT : PLAYER_LIMIT + 1);
                int list_height = item_count * (FontHeight + FontYSpacing - 1) + 2;
                RosterDropListClass* list = new RosterDropListClass(SLOT_FIRST + row * 4 + col,
                    labels[row][col], sizeof(labels[row][col]), text,
                    xs[col] * factor, (42 + row * 10) * factor,
                    widths[col] * factor - Get_Build_Frame_Width(down), list_height, up, down);
                list->Set_Read_Only(true);
                if (col == 0) {
                    if (row != 0) {
                        list->Add_Item("Random");
                    }
                    list->Add_Item("GDI");
                    list->Add_Item("Nod");
                } else if (col == 1) {
                    for (char const* color : colors) {
                        list->Add_Item(color);
                    }
                } else {
                    for (char const* team : teams) {
                        list->Add_Item(team);
                    }
                }
                dropdowns[row * 3 + col] = list;
                slots[row][col].reset(list);
            } else {
                slots[row][col].reset(new TextButtonClass(SLOT_FIRST + row * 4 + col, labels[row][col], button_text,
                    xs[col] * factor, (42 + row * 10) * factor, widths[col] * factor, 9 * factor));
            }
        }
    }
    int linked_count = 0;
    bool redraw = true;
    bool done = false;
    bool accepted = false;
    std::string status;
    static char const* const difficulties[] = {"Easy", "Easier", "Normal", "Harder", "Hard"};
    Load_Title_Screen(TitlePicture, &HidPage, Palette);
    Blit_Hid_Page_To_Seen_Buff();
    Set_Palette(Palette);
    while (Get_Mouse_State() > 0) {
        Show_Mouse();
    }

    while (!done) {
        Call_Back();
        if (AllSurfaces.SurfacesRestored) {
            AllSurfaces.SurfacesRestored = false;
            redraw = true;
        }
        if (redraw) {
            selected = map_list.Current_Index();
            MapInfo const& map = maps[selected];
            settings.Scenario = map.Scenario;
            int capacity = map.Players;
            if (settings.Count > capacity && capacity > 0) {
                settings.Count = capacity;
                status = "Extra AI removed to fit this map.";
            }
            if (linked_count != settings.Count) {
                for (int row = 0; row < PLAYER_LIMIT; ++row) {
                    for (int col = 0; col < 4; ++col) {
                        slots[row][col]->Remove();
                        if (row < settings.Count && (col != 3 || row != 0)) {
                            slots[row][col]->Add_Tail(*commands);
                        }
                    }
                }
                linked_count = settings.Count;
            }
            for (int row = 0; row < settings.Count; ++row) {
                Slot const& slot = settings.Slots[row];
                dropdowns[row * 3]->Set_Selected_Index(slot.Faction + (row == 0 ? 0 : 1));
                dropdowns[row * 3 + 1]->Set_Selected_Index(slot.Color);
                dropdowns[row * 3 + 2]->Set_Selected_Index(slot.Team);
                std::sprintf(labels[row][3], "-");
            }
            credits.Set_Maximum(10000);
            credits.Set_Value(settings.Credits);
            units.Set_Maximum(MPlayerCountMax[settings.Bases] - MPlayerCountMin[settings.Bases]);
            units.Set_Value(settings.Units - MPlayerCountMin[settings.Bases]);
            tech.Set_Maximum(MPLAYER_BUILD_LEVEL_MAX - 1);
            tech.Set_Value(settings.TechLevel - 1);
            rules.Check_Item(0, settings.Bases);
            rules.Check_Item(1, settings.Tiberium);
            rules.Check_Item(2, settings.Crates);
            rules.Check_Item(3, settings.CaptureTheFlag);
            difficulty.Set_Text(difficulties[settings.Difficulty]);
            if (settings.Count < capacity) {
                add.Enable();
            } else {
                add.Disable();
            }
            if (settings.Has_Opponent() && capacity >= settings.Count) {
                start.Enable();
            } else {
                start.Disable();
            }

            Hide_Mouse();
            Dialog_Box(5 * factor, 3 * factor, 310 * factor, 194 * factor);
            auto print = [&](char const* value, int x, int y) {
                Fancy_Text_Print(value, x * factor, y * factor, CC_GREEN, TBLACK, text);
            };
            Fancy_Text_Print("SKIRMISH", 160 * factor, 7 * factor, CC_GREEN, TBLACK, text | TPF_CENTER);
            print("Name:", 10, 20);
            print("Difficulty changes bonuses", 169, 20);
            print("Maps - smallest first", 10, 33);
            print("Player", 158, 33);
            print("Faction", 182, 33);
            print("Colour", 219, 33);
            print("Team", 269, 33);
            for (int row = 0; row < settings.Count; ++row) {
                char label[16];
                if (row == 0) {
                    std::sprintf(label, "You");
                } else {
                    std::sprintf(label, "AI %d", row);
                }
                print(label, 158, 44 + row * 10);
            }
            char info[80];
            std::sprintf(info, "%dx%d - %d players max", map.Width, map.Height, map.Players);
            print(info, 10, 115);
            if (!status.empty()) {
                print(status.c_str(), 158, 115);
            }
            print("Starting credits", 10, 128);
            print("Starting units", 10, 140);
            print("Tech level", 10, 152);
            print("Match difficulty", 10, 168);
            print("Choose fields from lists", 169, 166);
            print("None = free-for-all", 169, 174);
            std::sprintf(info, "%d", settings.Credits);
            print(info, 127, 128);
            std::sprintf(info, "%d", settings.Units);
            print(info, 127, 140);
            std::sprintf(info, "%d", settings.TechLevel);
            print(info, 127, 152);
            commands->Draw_All(true);
            for (RosterDropListClass* list : dropdowns) {
                if (list->IsDropped) {
                    list->List.Draw_Me(true);
                }
            }
            Show_Mouse();
            redraw = false;
        }

        bool was_dropped[PLAYER_LIMIT * 3];
        bool any_dropped = false;
        for (int index = 0; index < PLAYER_LIMIT * 3; ++index) {
            was_dropped[index] = dropdowns[index]->IsDropped;
            any_dropped |= was_dropped[index];
        }
        // Escape dismisses the popup, not the whole setup screen. Consume it
        // before ListClass can translate it into a selection notification.
        if (any_dropped && Keyboard->Check() == KN_ESC) {
            Keyboard->Get();
            for (RosterDropListClass* list : dropdowns) {
                list->Collapse();
            }
            redraw = true;
            Frame_Limiter();
            continue;
        }
        KeyNumType input = commands->Input();
        if (input & KN_BUTTON) {
            int id = input & ~KN_BUTTON;
            status.clear();
            redraw = true;
            if (id >= SLOT_FIRST && id < SLOT_FIRST + PLAYER_LIMIT * 4) {
                int row = (id - SLOT_FIRST) / 4;
                int col = (id - SLOT_FIRST) % 4;
                if (row < settings.Count) {
                    switch (col) {
                    case 0:
                        settings.Slots[row].Faction = dropdowns[row * 3]->Current_Index() - (row == 0 ? 0 : 1);
                        break;
                    case 1:
                        settings.Select_Color(row, dropdowns[row * 3 + 1]->Current_Index());
                        break;
                    case 2:
                        settings.Slots[row].Team = dropdowns[row * 3 + 2]->Current_Index();
                        break;
                    case 3:
                        settings.Remove(row);
                        break;
                    }
                }
            } else {
                switch (id) {
                case ADD_AI:
                    if (settings.Count < maps[selected].Players) {
                        ++settings.Count;
                    }
                    break;
                case CREDITS:
                    settings.Credits = credits.Get_Value();
                    break;
                case UNITS:
                    settings.Units = units.Get_Value() + MPlayerCountMin[settings.Bases];
                    break;
                case TECH:
                    settings.TechLevel = tech.Get_Value() + 1;
                    break;
                case RULES:
                    settings.Bases = rules.Is_Checked(0);
                    settings.Tiberium = rules.Is_Checked(1);
                    settings.Crates = rules.Is_Checked(2);
                    settings.CaptureTheFlag = rules.Is_Checked(3);
                    settings.Normalize(MPLAYER_BUILD_LEVEL_MAX);
                    break;
                case DIFFICULTY:
                    settings.Difficulty = (settings.Difficulty + (Rule.IsFineDifficulty ? 1 : 2))
                                          % (Rule.IsFineDifficulty ? 5 : 6);
                    break;
                case DEFAULTS:
                    settings = Settings();
                    settings.Name = name;
                    map_list.Set_Selected_Index(0);
                    break;
                case START:
                    if (settings.Has_Opponent() && settings.Count <= maps[selected].Players) {
                        accepted = done = true;
                    }
                    break;
                case BACK:
                    done = true;
                    break;
                }
            }
        } else if (input == KN_ESC) {
            done = true;
        }
        // One popup at a time; repaint the rows it covered when dismissed.
        int opened = -1;
        for (int index = 0; index < PLAYER_LIMIT * 3; ++index) {
            if (!was_dropped[index] && dropdowns[index]->IsDropped) {
                opened = index;
            }
        }
        for (int index = 0; index < PLAYER_LIMIT * 3; ++index) {
            RosterDropListClass& list = *dropdowns[index];
            if ((input & KN_BUTTON) || input == KN_LMOUSE || input == KN_RMOUSE
                || (opened >= 0 && index != opened)) {
                list.Collapse();
            }
            if (was_dropped[index] != static_cast<bool>(list.IsDropped)) {
                redraw = true;
            }
            if (list.IsDropped) {
                list.List.Draw_Me(true);
            }
        }
        Frame_Limiter();
    }

    settings.Name = name;
    if (settings.Name.empty()) {
        settings.Name = "Noname";
    }
    if (!Save_Settings(settings)) {
        WWMessageBox().Process("Could not save skirmish settings to conquer.ini.");
    }
    if (accepted) {
        Match = settings;
        MatchActive = true;
        MPlayerCount = 1;
        MPlayerGhosts = settings.Count - 1;
        MPlayerHouse = static_cast<HousesType>(HOUSE_GOOD + settings.Slots[0].Faction);
        MPlayerPrefColor = MPlayerColorIdx = static_cast<PlayerColorType>(settings.Slots[0].Color);
        std::snprintf(MPlayerName, sizeof(MPlayerName), "%s", settings.Name.c_str());
        strcpy(MPlayerNames[0], MPlayerName);
        MPlayerLocalID = MPlayerID[0] = Build_MPlayerID(MPlayerColorIdx, MPlayerHouse);
        MPlayerCredits = settings.Credits;
        MPlayerUnitCount = settings.Units;
        MPlayerBases = settings.Bases;
        MPlayerTiberium = settings.Tiberium;
        MPlayerGoodies = settings.Crates;
        Special.IsTGrowth = Special.IsTSpread = settings.Tiberium;
        Special.IsCaptureTheFlag = settings.CaptureTheFlag;
        BuildLevel = settings.TechLevel;
        Scen.Scenario = settings.Scenario;
        ScenarioIdx = maps[selected].OriginalIndex;
        Set_Difficulty(settings.Difficulty);
        std::srand(static_cast<unsigned>(std::time(NULL)));
        Seed = std::rand();
    }
    Hide_Mouse();
    Load_Title_Screen(TitlePicture, &HidPage, Palette);
    Blit_Hid_Page_To_Seen_Buff();
    Show_Mouse();
    return accepted;
}

void Assign_Skirmish_Houses()
{
    if (!MatchActive || MPlayerCount != 1 || Match.Count != MPlayerGhosts + 1) {
        return;
    }
    Set_Difficulty(Match.Difficulty);
    for (int i = 0; i < Match.Count; ++i) {
        HouseClass* house = HouseClass::As_Pointer(static_cast<HousesType>(HOUSE_MULTI1 + i));
        if (!house) {
            continue;
        }
        // Retain the engine's seeded random faction when this slot is Random.
        HousesType faction = Match.Slots[i].Faction == -1 ? house->ActLike
            : static_cast<HousesType>(HOUSE_GOOD + Match.Slots[i].Faction);
        house->Init_Data(static_cast<PlayerColorType>(Match.Slots[i].Color), faction, MPlayerCredits);
        house->Assign_Handicap(i == 0 ? Scen.Difficulty : Scen.CDifficulty);
        if (i != 0) {
            std::sprintf(house->Name, "AI %d", i);
        }
        for (int j = 0; j < Match.Count; ++j) {
            if (Match.Allied(i, j)) {
                house->Make_Ally(static_cast<HousesType>(HOUSE_MULTI1 + j));
            }
        }
    }
}

// Optional, tagged extension: old recordings retain their original event-stream offset.
// Never consult the user's current lobby config when replaying a match.
void Save_Skirmish_Recording(FileClass& file)
{
    if (GameToPlay != GAME_SKIRMISH || !MatchActive) {
        return;
    }
    int values[3 + TDSkirmish::PLAYER_LIMIT * 3] = {0x314b534b, Match.Count, Match.Difficulty};
    for (int i = 0; i < TDSkirmish::PLAYER_LIMIT; ++i) {
        values[3 + i * 3] = Match.Slots[i].Faction;
        values[4 + i * 3] = Match.Slots[i].Color;
        values[5 + i * 3] = Match.Slots[i].Team;
    }
    file.Write(values, sizeof(values));
}

void Load_Skirmish_Recording(FileClass& file)
{
    MatchActive = false;
    if (GameToPlay != GAME_SKIRMISH) {
        return;
    }
    int position = file.Seek(0, SEEK_CUR);
    int values[3 + TDSkirmish::PLAYER_LIMIT * 3];
    if (file.Read(values, sizeof(values)) != sizeof(values) || values[0] != 0x314b534b) {
        file.Seek(position, SEEK_SET);
        return;
    }
    Match = TDSkirmish::Settings();
    Match.Count = values[1];
    Match.Difficulty = values[2];
    for (int i = 0; i < TDSkirmish::PLAYER_LIMIT; ++i) {
        Match.Slots[i] = TDSkirmish::Slot{values[3 + i * 3], values[4 + i * 3], values[5 + i * 3]};
    }
    Match.Normalize(MPLAYER_BUILD_LEVEL_MAX);
    MatchActive = Match.Count == MPlayerGhosts + 1;
}
#endif // !REMASTER_BUILD
