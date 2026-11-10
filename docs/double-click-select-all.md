# Standalone double-click selection

Implemented for TD and RA in `91d579a`; retained when the divergent Windows/MacBook work was merged in `14d8c6d`. This document replaces the earlier handoff plan. The older simulation-frame-based detector from `0fd1706` is no longer used.

## Behavior

- Double-click a player-owned, selectable combat unit to select all on-screen units of that exact type.
- Includes infantry, vehicles, aircraft, and RA vessels; excludes buildings.
- Replaces the selection unless Shift/the configured selection modifier is held, in which case it adds matching units.
- Filters enemy, cloaked, undiscovered, non-selectable, and off-screen objects.

This is a **standalone UI feature**. `REMASTER_BUILD` excludes its selection helper, click-tracking fields, initialization, and release handler, and retains the original mouse-release signature/call sites. GlyphX supplies the Remastered client's selection behavior.

## Input pipeline

1. **Backend:** `common/wwkeyboard_sdl2.cpp` counts left-button presses within `<500 ms` and 32 pixels per axis. It synthesizes this metadata because SDL2 compatibility layers do not reliably forward SDL3's click count. The count is carried on both press and release events.
2. **Keyboard buffer:** `Put_Mouse_Message` in `common/wwkeyboard.cpp/.h` carries a double-click flag. `Has_Native_Double_Click()` tells the game whether to trust backend detection or use its fallback.
3. **Gadgets:** `common/gadget.cpp/.h` dispatches `LEFTDBLCLICK`. `GadgetClass::Clicked_On` preserves that metadata while masking events against the gadget's accepted left-button flags.
4. **Tactical display:** each game's `display.cpp` forwards the flag to `Mouse_Left_Release`. This checks selection-compatible actions, including `ACTION_SELF` and **`ACTION_NONE`**: a second click on an already-selected unit may otherwise be discarded. RA also accepts `ACTION_NOMOVE`, `ACTION_MOVE`, and `ACTION_NO_DEPLOY` when clicking the sole selected unit, outside repair/sell/superweapon targeting modes and without force-move/force-attack modifiers. RA's tank and aircraft overrides can translate `ACTION_SELF` into these actions before the release handler sees it; TD's ordinary tanks instead return `ACTION_NONE`.
5. **Selection:** `DisplayClass::Select_All_Type` compares `Techno_Type_Class()` pointers and filters eligible objects. The ground-layer and aircraft-heap walk is `Select_Matching_On_Screen` in `common/selectscan.h`. Each game passes its own filter and screen point. TD adjusts aircraft-heap positions for `Altitude` and still uses `Render_Coord` for the ground layer, including landed aircraft. RA aircraft use `Center_Coord()`; that scan does not subtract `Height`, which is what drawing does. Buildings are rejected by the filter, so RA vessels stay included. Box selection remains `Select_These` in each `display.cpp` and still runs in Remastered builds.

For backends without native detection, the display checks the last type and elapsed `WinTickCount.Time()` against `DOUBLE_CLICK_TIME` (currently 45 ticks). It does **not** use the simulation `Frame` counter. Tracking resets in the constructor and `Init_Clear`; a successful double-click consumes/reset its tracking state rather than dispatching the unit's normal self-action.

The SDL2 distance check uses window coordinates. Positions delivered to the engine separately honor video scaling and raw-input/gamepad handling; do not conflate these coordinate systems.

## Files to inspect when debugging

- `common/wwkeyboard.h`, `wwkeyboard.cpp`, `wwkeyboard_sdl2.cpp/.h`
- `common/gadget.cpp/.h`
- `common/selectscan.h`
- `tiberiandawn/display.cpp/.h`
- `redalert/display.cpp/.h`

Check the entire chain before tuning the timer. A lost gadget flag or an excluded release action can look like unreliable timing even when backend detection works correctly.

## Regression checklist

- Single click selects one unit; clicking an already-selected unit still permits double-click selection.
- RA: repeat with ordinary tanks (`ACTION_NOMOVE`), aircraft (`ACTION_MOVE`/`ACTION_NOMOVE`), and an MCV with blocked deployment (`ACTION_NO_DEPLOY`). Force-move/force-attack and targeting modes must not trigger the new self-selection cases.
- Double-click selects matching on-screen units, not other types or buildings.
- Shift+double-click adds to an existing mixed selection.
- Enemy, cloaked, undiscovered, and off-screen units remain unselected.
- Aircraft are included; RA vessels are included.
- Box selection, Shift-click, right-click cancel, and unit-specific self-actions work normally outside the double-click case.
- Rally points and service queues still work.
- Check both games and affected backends, especially SDL2 on macOS with `sdl2-compat`.
- Compile both standalone games and both Remastered DLLs when changing shared declarations or build guards.

The Windows TD Debug standalone build was manually tested by the maintainer on 2026-11-09, who confirmed double-click selection works. That is not a claim that every checklist case, RA, or every backend has been manually verified.
