# Red Alert engine

Read the root guide first. Mirror shared fixes into TD when appropriate, but do not copy its identifiers or mode/altitude fields verbatim.

## Where to look

| Concern | Entry points |
| --- | --- |
| Selection/input | `display.cpp/.h`, shared keyboard/gadget code |
| Production/build limits/base AI | `house.cpp/.h`, `sidebar.cpp`, `factory.cpp` |
| Rally/service docks/production exits | `building.cpp/.h`, `event.cpp/.h`, `drive.cpp`, `foot.cpp`, `techno.cpp`, `unit.cpp` |
| Saboteur and commando behavior | `defines.h`, `idata.cpp`, `infantry.cpp`, `techno.cpp`, `house.cpp`, `dllinterface.cpp` |
| Aircraft scouting | `aadata.cpp`, `aircraft.cpp` |
| Nuke damage | `anim.cpp` (`Do_Atom_Damage`) |

## Mode and unit differences

- RA uses `Session.Type`; `GAME_NORMAL` is campaign. Preserve campaign Tanya/hero behavior when changing the skirmish/MP commando split or limits.
- Aircraft use **`Height`**, not TD's `Altitude`. `Look()` and firing reveals use the type's `SightRange`; do not reintroduce the old grounded-aircraft clamp to one cell (`fcc9076`). Stock `Sight=0` entries are raised to three in `RulesClass::Objects` after each INI read; non-zero INI rules override the default.
- RA selection includes vessels; buildings remain excluded. Double-click helper/state/handler are standalone-only, and the `ACTION_NONE` release case matters for already-selected units.
- Chronosphere targeting deliberately accepts eligible non-friendly units; keep the remaining type/deploy/transport restrictions (`c091c01`).

## Saboteur: more than a type-table entry

The current Soviet skirmish/MP commando is **Saboteur**, not the earlier Volkov prototype (`2cd7349`). References to Volkov in comments can describe reused Counterstrike behavior rather than the current name.

- `INFANTRY_SABOTEUR` is distinct from `INFANTRY_GENERAL`, but the Saboteur reuses the **GNRL** body/name asset path. `[GNRL]` INI handling can collide; the cameo override checks `[SABOTEUR]` first.
- `Is_Commando_Type` in `defines.h` groups Tanya and Saboteur. Audit AI production, C4/capture, firing, and retaliation hooks when changing commando behavior. Keep Tanya-only voice/laugh/death presentation restricted to Tanya.
- Initialization **and** `InfantryTypeClass::Read_INI` reapply skirmish/MP overrides. Editing only the initial table can be undone by rules loading. The silenced `Sniper` weapon uses `Burst = 2`, `VOC_SILENCER`, and no secondary weapon.
- `FullName` alone does not control presentation: `Full_Name()` and nominal/civilian-name logic also matter. Current code returns the existing `TXT_SABOTEUR` ID; do not assume an arbitrary new string ID is present in the host's language data.
- Classic cameo loading tries `SBTRICON`, `SABICON`, then `GNRLICON`, with Tanya fallback. Preserve both loose-file (`CCFileClass`/`Load_Alloc_Data`) and MIX lookup paths.
- Remastered sidebar export uses an **asset name**, not just the classic SHP pointer. It currently falls back to `E7ICON`; a custom classic SHP does not automatically create an HD host asset. See `resources/AGENTS.md`.
- `HouseClass::Can_Build` enforces one of the requested commando type per house in non-campaign modes. Do not rely on a sidebar change to enforce limits.

## Rally, repair, and service queues

- Rally commands use `OutList`/`EventClass::RALLY`; `Refresh_Rally` manages cell flags and selection-only visibility. Preserve redraws when a rally is set, moved, deselected, or destroyed, and leave foreign flags alone.
- Use `ANIM_FLAG`/`FLAGFLY` through cell state, not line rendering: Remastered interprets the line intercept as a laser effect. Dynamic-map export must filter opponent flags; marker placement must not block foundations.
- Repair bays, refineries, and helipads are queueable. **Airstrip rearming is intentionally not**: it has a different landing/multi-slot workflow.
- Dock queues store `TARGET` handles. `Dock_Advance` prunes invalid/stale heads; `RADIO_DOCKING` polling promotes the waiting head. Maintain detach/limbo/termination cleanup and queue capacity checks.
- Repair missions must target `STRUCT_REPAIR`, not the refinery (`88acdf1`). Dock finders prefer free/shorter queues; preserve that preference when altering availability tests.
- A helipad can be physically occupied even without current radio contact. Check active, non-limbo aircraft with `Height == 0` on the docking cell before production exits (`c091c01`).

## Standalone queues and nuke balance

Production queues are distinct from service queues. Compile their `HouseClass` state/methods, initialization, factory enqueue/advance paths, and sidebar changes only for standalone; preserve GlyphX's original busy-factory behavior for `REMASTER_BUILD`. Right-clicking a non-active cameo emits `CANCEL_QUEUE` only for a matching waiting entry; `Cancel_Queued_Production` removes the newest match without touching active production. Keep `EventLength`/`EventNames` synchronized with the event enum.

SDL2 explosion shake uses the shared presentation-only `Request_Video_Shake` path. Keep the blocking snapshot/`Frame_Limiter` loop only for non-SDL2 backends, and preserve the `REMASTER_BUILD` house timers.

Standalone Game Controls treats Esc like its confirmation action so game/scroll sliders are applied and saved. Sound Controls calls `Options.Save_Settings()` when closed, matching TD; do not rely on the outer options dialog to persist sound changes. Standalone shutdown also saves the complete `OptionsClass` state before the separate video/mouse `SettingsClass` save. `OptionsClass::Load_Settings` and `Save_Settings` explicitly use `Paths.User_Path()/redalert.ini`; do not resolve writable options back through the asset search path.

`Do_Atom_Damage` in `anim.cpp` uses a separate campaign/non-campaign branch. Current non-campaign behavior uses radius `3 * CELL_LEPTON_W`, damage `(Rule.AtomDamage * 3) / 4`, and `WARHEAD_NUKE`; keep campaign values separate and update `docs/komkommer.md` with balance edits.

Build target: `KomkommerRA` (standalone) or `RedAlert` (DLL). Test Allies/Soviets and campaign/skirmish separately; local custom maps can follow the campaign code path.
