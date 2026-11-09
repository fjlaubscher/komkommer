# Plan: double-click selects all units of the same type (QoL)

Status: **ready for implementation** · Handoff plan

## Goal

Add a quality-of-life behavior: **double-clicking a selectable combat unit selects
every on-screen unit of that exact type** (e.g. double-click one rifle infantry →
all on-screen rifle infantry). This matches Tiberian Sun / RA2 / OpenRA; it is *not*
in the 1995/1996 originals, so this is a deliberate mod deviation.

## Confirmed starting point

This feature **does not exist in the repo today**. Do not go hunting for it — a full
`git grep -i double` plus a `git diff origin/vanilla...main` of the selection files
turned up nothing. The only double-click code anywhere is:

- `mapedit.cpp` — the **map editor**'s own double-click (see below; it is the idiom to copy)
- `netdlg.cpp` / `teamtype.cpp` — WOL / team dialogs
- `common/wwkeyboard.cpp` — **Windows-only** `WM_LBUTTONDBLCLK` translation

SDL2 (the macOS/Linux/BSD path) never emits a double-click event, so it today arrives
as two ordinary left-clicks → select, then re-toggle the one unit. That is the
"flakiness": there is nothing for the second click to hook into.

## Decisions to confirm before/while implementing

1. **Games**: both Tiberian Dawn and Red Alert (mirror each other). Default: yes, both.
2. **Scope**: vanilla standalone binaries. The remaster DLL has a separate input path
   (`DisplayClass::TacticalClass::Selection_At_Mouse` / `Command_Object`, guarded by
   `#ifdef REMASTER_BUILD`) — leave it for a follow-up unless asked.
3. **What "same type" includes**: infantry, vehicles, aircraft (and RA vessels).
   **Buildings excluded** by default (double-click is for strike forces). Flag in review
   if buildings should also match.
4. **Selection result**: on double-click, replace current selection with all matching
   on-screen units (clears any non-matching selection). If **Shift** is held, add to the
   existing selection instead (consistent with existing shift-toggle).
5. **Timer window**: start at **30 game ticks** (~0.5 s at the 60 Hz rate `WinTickCount`
   uses). The map editor uses 15 ticks for context; 30 is a comfortable double-click.
   Make it a `#define` / named constant so it's easy to tune.

## Exact input flow (file:line anchors)

1. `common/wwkeyboard_sdl2.cpp` (`case SDL_MOUSEBUTTONDOWN/UP`, ~lines 58–84) maps
   `SDL_BUTTON_LEFT` → `VK_LBUTTON` and calls `Put_Mouse_Message(key, x, y, release)`.
   SDL1 equivalent is `common/wwkeyboard_sdl1.cpp`. No double-click synthesis here — the
   game must detect it by timing, from these key events.
2. The left-click **release** is the workhorse:
   - `tiberiandawn/display.cpp:3863` `DisplayClass::Mouse_Left_Release(CELL cell, int x, int y, ObjectClass* object, ActionType action, bool wwsmall)`
   - `redalert/display.cpp:3826` — same signature (RA). **The feature belongs here.**
3. `Select_These` (rubber-band box select) is the template for the "iterate and select"
   loop:
   - `tiberiandawn/display.cpp:3002` `DisplayClass::Select_These(COORDINATE coord1, COORDINATE coord2, bool additive)`
   - `redalert/display.cpp:~2807` (comment) / function just below.

## Reference idiom to copy (the map editor's double-click)

- `tiberiandawn/mapedit.cpp:1102`
  `if (CurrentObject.Count() && ((WinTickCount.Time() - LastClickTime) < 15))`
- member: `tiberiandawn/mapedit.h:290` `unsigned int LastClickTime;`
- RA equivalents: `redalert/mapedit.cpp:1044`, `redalert/mapedit.h:288`

## APIs available (verified)

- `object->Is_Techno()` — true for infantry/unit/aircraft(+vessel).
- `((TechnoClass*)object)->Techno_Type_Class()` → `TechnoTypeClass const*` (TD
  `techno.h:242`, RA `techno.h:304`). **Compare pointers for "same type".**
- `object->What_Am_I()` → `RTTIType` (`RTTI_INFANTRY`, `RTTI_UNIT`, `RTTI_AIRCRAFT`,
  `RTTI_BUILDING`, and RA `RTTI_VESSEL`).
- `object->Class_Of().IsSelectable`, `object->Is_Discovered_By_Player()`,
  `object->Is_Cloaked(PlayerPtr)`, `object->Is_Selected_By_Player()`, `object->Select()`.
- Ownership: only select objects owned by the local player (match how `Select_These`
  scopes to the player's view/ownership — reuse its exact discovery/cloak conditions).
- Timer: `WinTickCount.Time()`.

## Implementation sketch

### Add state to `DisplayClass` (both display.h files)

```cpp
unsigned int LastClickTime;          // from WinTickCount
TechnoTypeClass const* LastClickType; // nullptr after non-select / select
```

Reset both in the constructors / `Init_Clear`-style paths so a stale value can't leak
across scenarios.

### Hook into `Mouse_Left_Release` (in the select branch)

In the `action == ACTION_SELECT || (ACTION_NONE && selectable && !selected)` branch
(TD ~lines 3920–3937; equivalent in RA):

```cpp
if (object && object->Is_Techno()) {
    TechnoTypeClass const* type = ((TechnoClass*)object)->Techno_Type_Class();
    bool isDouble = (type == LastClickType
                     && (WinTickCount.Time() - LastClickTime) <= DOUBLE_CLICK_TIME);
    if (isDouble) {
        Select_All_Type(type, /*additive=*/shiftHeld);
        LastClickTime = 0;
        LastClickType = nullptr;
        // skip the normal single-select below
    } else {
        // << existing single-click select logic unchanged >>
        LastClickTime = WinTickCount.Time();
        LastClickType = type;
    }
}
```

Preserve the current unit-below-cursor semantics for the non-double path exactly as-is
(so rally points, dock queue, commando limits, and box-select are untouched).

### Add a helper `DisplayClass::Select_All_Type(TechnoTypeClass const* type, bool additive)`

Model it on `Select_These`:

1. If `!additive`, `Unselect_All()`.
2. Iterate the same layers `Select_These` uses (ground + air), and for each object:
   - `obj->Is_Techno()`
   - `obj->Techno_Type_Class() == type`
   - `obj->Class_Of().IsSelectable`
   - owned by the local player, `Is_Discovered_By_Player()`, not cloaked (reuse
     `Select_These`'s exact conditions)
   - call `obj->Select()`.

Do **not** select buildings (excluded from the type match by using the techno check and
the same-type pointer).

## Files to change

- `tiberiandawn/display.h` + `tiberiandawn/display.cpp`
- `redalert/display.h` + `redalert/display.cpp`
- docs only: add one bullet under "Both games" in `docs/komkommer.md` noting this QoL
  deviation (so the "mod changes" doc stays accurate).

## Verification checklist

- [ ] `cmake --build --preset release` (see `scripts/package-macos.sh` and
      `CMakePresets.json`; on macOS configure needs
      `-DCMAKE_PREFIX_PATH="/opt/homebrew;/opt/homebrew/opt/openal-soft"`).
- [ ] Place N units of one type + 1 unit of another type, all owned by the player.
- [ ] Single click selects exactly the one unit.
- [ ] Double click selects all on-screen units of that type, and only that type.
- [ ] A same-type unit that is off-screen / cloaked / undiscovered is **not** selected.
- [ ] Shift+double-click adds to an existing selection instead of replacing.
- [ ] Box-select (drag), single click, shift-click, and right-click cancel are unchanged.
- [ ] Existing mod behaviors still work: rally points, dock queues, commando 1-per-house.
- [ ] Both TD and RA behave the same.

## Definition of done

Double-click selects all on-screen units of the exact type across both games, without
regressing any existing selection or the mod's other behaviors; the new behavior is noted
in `docs/komkommer.md`; timer is a named/tunable constant, not a magic number.
