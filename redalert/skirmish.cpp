#ifndef REMASTER_BUILD
#include <algorithm>
#include <cctype>
#include <cstdlib>
#include <ctime>
#include <memory>
#include <vector>
#include "function.h"
#include "drop.h"
#include "cheklist.h"
#include "textbtn.h"
#include "msgbox.h"
#include "framelimit.h"
#include "paths.h"
#include "skirmish.h"
#include "skirmishsettings.h"
#include "skirmishlayout.h"
#include "common/skirmishdrop.h"

extern bool Is_Mission_Counterstrike(char* filename);
extern bool Is_Mission_Aftermath(char* filename);

namespace
{
using namespace RASkirmish;
using RosterDropListClass = SkirmishDropListClass<DropListClass>;
static_assert(PLAYER_LIMIT == MAX_MPLAYER_COLORS, "RA requires one colour per lobby slot");
static_assert(COUNTRY_COUNT == HOUSE_FRANCE - HOUSE_USSR + 1, "Keep the original playable countries");
Settings Match;
bool MatchActive = false;

void Set_Difficulty(int difficulty)
{
    static DiffType const human[] = {DIFF_EASY, DIFF_NORMAL, DIFF_NORMAL, DIFF_NORMAL, DIFF_HARD};
    static DiffType const computer[] = {DIFF_HARD, DIFF_HARD, DIFF_NORMAL, DIFF_EASY, DIFF_EASY};
    Scen.Difficulty = human[difficulty];
    Scen.CDifficulty = computer[difficulty];
}

Settings Defaults()
{
    Settings settings;
    settings.Credits = Rule.MPDefaultMoney;
    settings.Bases = Rule.IsMPBasesOn;
    settings.Ore = Rule.IsMPTiberiumGrow;
    settings.Crates = Rule.IsMPCrates;
    settings.Shadow = Rule.IsMPShadowGrow;
    settings.CaptureTheFlag = Rule.IsMPCaptureTheFlag;
    settings.Units = (SessionClass::CountMax[settings.Bases] + SessionClass::CountMin[settings.Bases]) / 2;
    settings.Normalize(Rule.MPMaxMoney, Rule.MaxPlayers);
    return settings;
}

std::string Config_Path()
{
    return PathsClass::Concatenate_Paths(Paths.User_Path(), CONFIG_FILE_NAME);
}

bool Save_Settings(Settings const& settings)
{
    INIClass ini;
    RawFileClass file(Config_Path().c_str());
    if (file.Is_Available() && !ini.Load(file)) {
        return false;
    }
    settings.Save(ini);
    return ini.Save(file) > 0;
}

enum Buttons
{
    NAME = 100,
    MAPS,
    ADD_AI,
    CREDITS,
    UNITS,
    TECH,
    RULES,
    EXTRA_RULES,
    DIFFICULTY,
    DEFAULTS,
    START,
    BACK,
    SLOT_FIRST = 200
};
}

int Skirmish_Dialog()
{
    using namespace RASkirmish;
    MatchActive = false;
    Set_Logic_Page(SeenBuff);
    int const factor = SeenBuff.Get_Width() == 320 ? 1 : 2;
    int const player_limit = Skirmish_Clamp(Rule.MaxPlayers, 1, PLAYER_LIMIT);
    TextPrintType const text = TPF_TEXT;
    RemapControlType* scheme = GadgetClass::Get_Color_Scheme();
    Settings settings = Defaults();
    settings.Name = Session.Handle;
    settings.Slots[0].Faction = Session.House - HOUSE_USSR;
    settings.Slots[0].Color = Session.PrefColor;
    INIClass config;
    RawFileClass config_file(Config_Path().c_str());
    if (config_file.Is_Available() && config.Load(config_file)) {
        settings.Load(config);
    }
    settings.Normalize(Rule.MPMaxMoney, player_limit);
    if (!Rule.IsFineDifficulty) {
        settings.Difficulty = (settings.Difficulty / 2) * 2;
    }

    std::vector<MapInfo> maps;
    for (int i = 0; i < Session.Scenarios.Count(); ++i) {
        MultiMission* mission = Session.Scenarios[i];
        char const* filename = mission->Get_Filename();
        // Retain the stock expansion-hosting restrictions, without filtering by list index.
        if (mission->Get_Official()
            && ((Is_Mission_Counterstrike(const_cast<char*>(filename)) && !Is_Counterstrike_Installed())
                || (Is_Mission_Aftermath(const_cast<char*>(filename)) && !Is_Aftermath_Installed()))) {
            continue;
        }
        CCFileClass file(filename);
        INIClass ini;
        if (!ini.Load(file)) {
            continue;
        }
        MapInfo map;
        map.Scenario = filename;
        map.OriginalIndex = i;
        map.Name = mission->Description();
        std::transform(map.Name.begin(), map.Name.end(), map.Name.begin(), [](unsigned char c) {
            return static_cast<char>(std::toupper(c));
        });
        map.Read(ini);
        maps.push_back(map);
    }
    if (maps.empty()) {
        WWMessageBox().Process("No skirmish maps are available.");
        return false;
    }
    std::sort(maps.begin(), maps.end());

    void const* up = MFCD::Retrieve("BTN-UP.SHP");
    void const* down = MFCD::Retrieve("BTN-DN.SHP");
    char name[MPLAYER_NAME_MAX];
    std::snprintf(name, sizeof(name), "%s", settings.Name.c_str());
    EditClass name_edit(NAME, name, sizeof(name), text,
                        Layout::Name.X * factor, Layout::Name.Y * factor,
                        Layout::Name.Width * factor, Layout::Name.Height * factor,
                        EditClass::ALPHANUMERIC);
    GadgetClass* commands = &name_edit;
    ListClass map_list(MAPS, Layout::MapList.X * factor, Layout::MapList.Y * factor,
                       Layout::MapList.Width * factor, Layout::MapList.Height * factor, text, up, down);
    map_list.Add_Tail(*commands);
    Fancy_Text_Print("", 0, 0, scheme, TBLACK, text);
    int selected = 0;
    for (int i = 0; i < static_cast<int>(maps.size()); ++i) {
        char suffix[16];
        std::sprintf(suffix, " (%dP)", maps[i].Players);
        std::string label = maps[i].Name;
        int width = (Layout::MapList.Width - 18) * factor - String_Pixel_Width(suffix);
        if (String_Pixel_Width(label.c_str()) > width) {
            while (!label.empty() && String_Pixel_Width((label + "...").c_str()) > width) {
                label.pop_back();
            }
            label += "...";
        }
        maps[i].Label = label + suffix;
        map_list.Add_Item(maps[i].Label.c_str());
        if (_stricmp(maps[i].Scenario.c_str(), settings.Scenario.c_str()) == 0) {
            selected = i;
        }
    }
    map_list.Set_Selected_Index(selected);

    TextButtonClass add(ADD_AI, "Add AI", TPF_BUTTON, Layout::AddAI.X * factor, Layout::AddAI.Y * factor,
                        Layout::AddAI.Width * factor, Layout::AddAI.Height * factor);
    TextButtonClass difficulty(DIFFICULTY, "", TPF_BUTTON, Layout::Difficulty.X * factor, Layout::Difficulty.Y * factor,
                               Layout::Difficulty.Width * factor, Layout::Difficulty.Height * factor);
    TextButtonClass defaults(DEFAULTS, "Restore defaults", TPF_BUTTON,
                             Layout::Defaults.X * factor, Layout::Defaults.Y * factor,
                             Layout::Defaults.Width * factor, Layout::Defaults.Height * factor);
    TextButtonClass start(START, "Start", TPF_BUTTON, Layout::Start.X * factor, Layout::Start.Y * factor,
                          Layout::Start.Width * factor, Layout::Start.Height * factor);
    TextButtonClass back(BACK, "Back", TPF_BUTTON, Layout::Back.X * factor, Layout::Back.Y * factor,
                         Layout::Back.Width * factor, Layout::Back.Height * factor);
    GaugeClass credits(CREDITS, Layout::Credits.X * factor, Layout::Credits.Y * factor,
                        Layout::Credits.Width * factor, Layout::Credits.Height * factor);
    GaugeClass units(UNITS, Layout::Units.X * factor, Layout::Units.Y * factor,
                      Layout::Units.Width * factor, Layout::Units.Height * factor);
    GaugeClass tech(TECH, Layout::Tech.X * factor, Layout::Tech.Y * factor,
                     Layout::Tech.Width * factor, Layout::Tech.Height * factor);
    CheckListClass rules(RULES, Layout::Rules.X * factor, Layout::Rules.Y * factor,
                         Layout::Rules.Width * factor, Layout::Rules.Height * factor, text, up, down);
    CheckListClass extra_rules(EXTRA_RULES, Layout::ExtraRules.X * factor, Layout::ExtraRules.Y * factor,
                               Layout::ExtraRules.Width * factor, Layout::ExtraRules.Height * factor, text, up, down);
    int tabs[] = {8};
    rules.Set_Tabs(tabs);
    rules.Set_Read_Only(false);
    extra_rules.Set_Tabs(tabs);
    extra_rules.Set_Read_Only(false);
    rules.Add_Item("Bases");
    rules.Add_Item("Ore regrows");
    rules.Add_Item("Crates");
    extra_rules.Add_Item("Shadow regrows");
    extra_rules.Add_Item("Capture the flag");
    GadgetClass* controls[] = {&add, &difficulty, &defaults, &start, &back, &credits, &units, &tech, &rules, &extra_rules};
    for (GadgetClass* control : controls) {
        control->Add_Tail(*commands);
    }

    char labels[PLAYER_LIMIT][4][32] = {};
    std::unique_ptr<GadgetClass> slots[PLAYER_LIMIT][4];
    RosterDropListClass* dropdowns[PLAYER_LIMIT * 3];
    // These are RA's standalone colour indices, not TD's remap enum.
    static char const* const colors[] = {"Gold", "Lt blue", "Red", "Green", "Orange", "Grey", "Blue", "Brown"};
    static char const* const countries[] = {"USSR", "England", "Ukraine", "Germany", "France"};
    static char const* const teams[] = {"None", "1", "2", "3", "4", "5", "6", "7", "8"};
    for (int row = 0; row < PLAYER_LIMIT; ++row) {
        for (int col = 0; col < 4; ++col) {
            if (col < 3) {
                int item_count = col == 0 ? COUNTRY_COUNT + (row != 0) : (col == 1 ? PLAYER_LIMIT : PLAYER_LIMIT + 1);
                int list_height = item_count * (FontHeight + FontYSpacing - 1) + 2;
                RosterDropListClass* list = new RosterDropListClass(SLOT_FIRST + row * 4 + col,
                    labels[row][col], sizeof(labels[row][col]), text,
                    Layout::ColumnX[col] * factor, (Layout::Roster.Y + row * Layout::RowPitch) * factor,
                    Layout::ColumnWidth[col] * factor - Get_Build_Frame_Width(down), list_height, up, down);
                list->Set_Read_Only(true);
                if (col == 0) {
                    if (row != 0) {
                        list->Add_Item("Random");
                    }
                    for (char const* country : countries) {
                        list->Add_Item(country);
                    }
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
                std::sprintf(labels[row][col], "-");
                slots[row][col].reset(new TextButtonClass(SLOT_FIRST + row * 4 + col, labels[row][col], TPF_BUTTON,
                    Layout::ColumnX[col] * factor, (Layout::Roster.Y + row * Layout::RowPitch) * factor,
                    Layout::ColumnWidth[col] * factor, Layout::RowPitch * factor));
            }
        }
    }
    int linked_count = 0;
    bool redraw = true;
    bool done = false;
    bool accepted = false;
    std::string status;
    static char const* const difficulties[] = {"Easy", "Easier", "Normal", "Harder", "Hard"};
    Load_Title_Page(true);
    CCPalette.Set();

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
            int capacity = Skirmish_Clamp(map.Players, 0, player_limit);
            if (settings.Count > capacity && capacity > 0) {
                settings.Count = capacity;
                status = "Extra AI removed to fit map.";
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
            }
            credits.Set_Maximum(Rule.MPMaxMoney > 0 ? Rule.MPMaxMoney : 1);
            credits.Set_Value(settings.Credits);
            if (Rule.MPMaxMoney <= 0) {
                credits.Disable();
            }
            units.Set_Maximum(SessionClass::CountMax[settings.Bases] - SessionClass::CountMin[settings.Bases]);
            units.Set_Value(settings.Units - SessionClass::CountMin[settings.Bases]);
            tech.Set_Maximum(MPLAYER_BUILD_LEVEL_MAX - 1);
            tech.Set_Value(settings.TechLevel - 1);
            rules.Check_Item(0, settings.Bases);
            rules.Check_Item(1, settings.Ore);
            rules.Check_Item(2, settings.Crates);
            extra_rules.Check_Item(0, settings.Shadow);
            extra_rules.Check_Item(1, settings.CaptureTheFlag);
            difficulty.Set_Text(difficulties[settings.Difficulty]);
            if (settings.Count < capacity) {
                add.Enable();
            } else {
                add.Disable();
            }
            if (settings.Has_Opponent() && settings.Count <= capacity) {
                start.Enable();
            } else {
                start.Disable();
            }

            Hide_Mouse();
            Dialog_Box(Layout::Dialog.X * factor, Layout::Dialog.Y * factor,
                       Layout::Dialog.Width * factor, Layout::Dialog.Height * factor);
            auto print = [&](char const* value, int x, int y) {
                Fancy_Text_Print(value, x * factor, y * factor, scheme, TBLACK, text);
            };
            Fancy_Text_Print("SKIRMISH", 160 * factor, 12 * factor, scheme, TBLACK, text | TPF_CENTER);
            print("Name:", Layout::Interior.X, Layout::Name.Y + 1);
            if (!status.empty()) {
                print(status.c_str(), Layout::Roster.X, Layout::Name.Y + 1);
            } else {
                print("Difficulty = bonuses", Layout::Roster.X, Layout::Name.Y + 1);
            }
            print("Maps - smallest first", Layout::MapList.X, Layout::HeaderY);
            print("Player", Layout::Roster.X, Layout::HeaderY);
            print("Country", Layout::ColumnX[0], Layout::HeaderY);
            print("Colour", Layout::ColumnX[1], Layout::HeaderY);
            print("Team", Layout::ColumnX[2], Layout::HeaderY);
            for (int row = 0; row < settings.Count; ++row) {
                char label[16];
                if (row == 0) {
                    std::sprintf(label, "You");
                } else {
                    std::sprintf(label, "AI %d", row);
                }
                print(label, Layout::Roster.X, Layout::Roster.Y + 2 + row * Layout::RowPitch);
            }
            char info[96];
            std::sprintf(info, "%dx%d - %d players max", map.Width, map.Height, capacity);
            print(info, Layout::MapInfo.X, Layout::MapInfo.Y);
            print("None = FFA", 204, Layout::AddAI.Y + 1);
            print("Credits", Layout::Roster.X, Layout::Credits.Y);
            print("Units", 232, Layout::Units.Y);
            print("Tech", Layout::Roster.X, Layout::Tech.Y);
            print("Difficulty", 225, Layout::Tech.Y);
            std::sprintf(info, "%d", settings.Credits);
            print(info, 206, Layout::Credits.Y);
            std::sprintf(info, "%d", settings.Units);
            print(info, 282, Layout::Units.Y);
            std::sprintf(info, "%d", settings.TechLevel);
            print(info, 206, Layout::Tech.Y);
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
        for (int i = 0; i < PLAYER_LIMIT * 3; ++i) {
            was_dropped[i] = dropdowns[i]->IsDropped;
            any_dropped |= was_dropped[i];
        }
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
                    if (settings.Count < maps[selected].Players && settings.Count < player_limit) {
                        ++settings.Count;
                    }
                    break;
                case CREDITS:
                    settings.Credits = credits.Get_Value();
                    break;
                case UNITS:
                    settings.Units = units.Get_Value() + SessionClass::CountMin[settings.Bases];
                    break;
                case TECH:
                    settings.TechLevel = tech.Get_Value() + 1;
                    break;
                case RULES:
                case EXTRA_RULES:
                    settings.Bases = rules.Is_Checked(0);
                    settings.Ore = rules.Is_Checked(1);
                    settings.Crates = rules.Is_Checked(2);
                    settings.Shadow = extra_rules.Is_Checked(0);
                    settings.CaptureTheFlag = extra_rules.Is_Checked(1);
                    settings.Normalize(Rule.MPMaxMoney, player_limit);
                    break;
                case DIFFICULTY:
                    settings.Difficulty = (settings.Difficulty + (Rule.IsFineDifficulty ? 1 : 2))
                                          % (Rule.IsFineDifficulty ? 5 : 6);
                    break;
                case DEFAULTS:
                    settings = Defaults();
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
        int opened = -1;
        for (int i = 0; i < PLAYER_LIMIT * 3; ++i) {
            if (!was_dropped[i] && dropdowns[i]->IsDropped) {
                opened = i;
            }
        }
        for (int i = 0; i < PLAYER_LIMIT * 3; ++i) {
            RosterDropListClass& list = *dropdowns[i];
            if ((input & KN_BUTTON) || input == KN_LMOUSE || input == KN_RMOUSE || (opened >= 0 && i != opened)) {
                list.Collapse();
            }
            if (was_dropped[i] != static_cast<bool>(list.IsDropped)) {
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
        WWMessageBox().Process("Could not save skirmish settings to redalert.ini.");
    }
    if (accepted) {
        Match = settings;
        MatchActive = true;
        Clear_Vector(&Session.Players);
        Session.NumPlayers = 1;
        Session.Options.AIPlayers = settings.Count - 1;
        Session.House = static_cast<HousesType>(HOUSE_USSR + settings.Slots[0].Faction);
        Session.PrefColor = Session.ColorIdx = static_cast<PlayerColorType>(settings.Slots[0].Color);
        std::snprintf(Session.Handle, sizeof(Session.Handle), "%s", settings.Name.c_str());
        NodeNameType* who = new NodeNameType;
        std::strcpy(who->Name, Session.Handle);
        who->Player.House = Session.House;
        who->Player.Color = Session.ColorIdx;
        who->Player.ProcessTime = -1;
        Session.Players.Add(who);
        Session.Options.Credits = settings.Credits;
        Session.Options.UnitCount = settings.Units;
        Session.Options.Bases = settings.Bases;
        Session.Options.Tiberium = settings.Ore;
        Session.Options.Goodies = settings.Crates;
        Special.IsTGrowth = Rule.IsTGrowth = Special.IsTSpread = Rule.IsTSpread = settings.Ore;
        Special.IsShadowGrow = settings.Shadow;
        Special.IsCaptureTheFlag = settings.CaptureTheFlag;
        BuildLevel = settings.TechLevel;
        Session.Options.ScenarioIndex = maps[selected].OriginalIndex;
        Scen.Scenario = maps[selected].OriginalIndex;
        std::snprintf(Scen.ScenarioName, sizeof(Scen.ScenarioName), "%s", settings.Scenario.c_str());
        Set_Difficulty(settings.Difficulty);
        std::srand(static_cast<unsigned>(std::time(NULL)));
        Seed = std::rand();
        VerNum.Init_Clipping();
    }
    Hide_Mouse();
    Load_Title_Page(true);
    Show_Mouse();
    return accepted;
}

void Assign_Skirmish_Houses()
{
    if (!MatchActive || Session.Players.Count() != 1 || Match.Count != Session.Options.AIPlayers + 1) {
        return;
    }
    Set_Difficulty(Match.Difficulty);
    for (int i = 0; i < Match.Count; ++i) {
        HouseClass* house = HouseClass::As_Pointer(static_cast<HousesType>(HOUSE_MULTI1 + i));
        if (!house) {
            continue;
        }
        HousesType country = Match.Slots[i].Faction == -1 ? house->ActLike
            : static_cast<HousesType>(HOUSE_USSR + Match.Slots[i].Faction);
        house->Init_Data(static_cast<PlayerColorType>(Match.Slots[i].Color), country, Session.Options.Credits);
        house->Assign_Handicap(i == 0 ? Scen.Difficulty : Scen.CDifficulty);
        if (i != 0) {
            std::sprintf(house->IniName, "AI %d", i);
        }
        unsigned allies = 0;
        for (int j = 0; j < Match.Count; ++j) {
            if (Match.Allied(i, j)) {
                allies |= 1u << (HOUSE_MULTI1 + j);
            }
        }
        house->Init_Skirmish_Allies(allies);
    }
}

void Save_Skirmish_Recording(FileClass& file)
{
    if (Session.Type != GAME_SKIRMISH || !MatchActive) {
        return;
    }
    int values[3 + RASkirmish::PLAYER_LIMIT * 3] = {0x3152534b, Match.Count, Match.Difficulty};
    for (int i = 0; i < RASkirmish::PLAYER_LIMIT; ++i) {
        values[3 + i * 3] = Match.Slots[i].Faction;
        values[4 + i * 3] = Match.Slots[i].Color;
        values[5 + i * 3] = Match.Slots[i].Team;
    }
    file.Write(values, sizeof(values));
}

void Load_Skirmish_Recording(FileClass& file)
{
    MatchActive = false;
    if (Session.Type != GAME_SKIRMISH) {
        return;
    }
    int position = file.Seek(0, SEEK_CUR);
    int values[3 + RASkirmish::PLAYER_LIMIT * 3];
    if (file.Read(values, sizeof(values)) != sizeof(values) || values[0] != 0x3152534b) {
        file.Seek(position, SEEK_SET);
        return;
    }
    Match = RASkirmish::Settings();
    Match.Count = values[1];
    Match.Difficulty = values[2];
    for (int i = 0; i < RASkirmish::PLAYER_LIMIT; ++i) {
        Match.Slots[i] = RASkirmish::Slot{values[3 + i * 3], values[4 + i * 3], values[5 + i * 3]};
    }
    Match.Normalize(Rule.MPMaxMoney, Rule.MaxPlayers);
    MatchActive = Match.Count == Session.Options.AIPlayers + 1;
}
#endif // !REMASTER_BUILD
