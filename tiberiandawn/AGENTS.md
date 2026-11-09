# Tiberian Dawn engine

Read the root guide first; shared gameplay concepts have a counterpart in `redalert/`, but TD uses different types and constants.

## Where to look

| Concern | Entry points |
| --- | --- |
| Selection/input and placement proximity | `display.cpp/.h` |
| Production, build eligibility, base AI | `house.cpp/.h`, `sidebar.cpp`, `factory.cpp`, `bdata.cpp` |
| Rally, docks, production exits | `building.cpp/.h`, `event.cpp/.h`, `drive.cpp`, `foot.cpp`, `techno.cpp`, `unit.cpp` |
| Aircraft scouting | `aadata.cpp`, `aircraft.cpp` |
| Nuke/Ion Cannon damage | `anim.cpp` |
| Remastered rendering/commands | `dllinterface.cpp/.h`, `sidebarglyphx.*` |

## TD-specific traps

- Mode checks use `GameToPlay == GAME_NORMAL` for campaign. Do not copy RA's `Session.Type` blindly.
- Aircraft use **`Altitude`**, not RA's `Height`. The double-click air sweep adjusts screen position using altitude; a ground-layer-only sweep misses flying units.
- Sight defaults in `aadata.cpp` are only part of scouting behavior. `aircraft.cpp` ground-layer entry and firing reveals must use `Class->SightRange`, not a hardcoded one-cell reveal (`fcc9076`).
- `HouseClass::Can_Build` has more than one eligibility path; preserve the `INFANTRY_RAMBO` one-per-house checks in both. Sidebar visibility alone is not enforcement.
- Nod Guard Tower/Repair Bay access involves ownership/prerequisites in `bdata.cpp` **and** the base AI in `house.cpp`. AI repair-bay consideration accepts a War Factory **or Airstrip**; defense choice has a faction-preferred tower plus fallback (`5bc064c`).
- Placement adjacency allows a one-cell gap from friendly buildings but must retain valid radar/foundation/proximity checks (`88acdf1`).
- TD standalone compiles with `MEGAMAPS`; the Remastered target deliberately omits it. Cell offsets and footprints are sensitive to map stride. Do not alter this as incidental cleanup.

## Rally and dock lifecycle

- `Can_Rally` is an explicit building whitelist, not every building that produces/services an object. The simulation command is `EventClass::RALLY`; update event names/lengths with event enum changes.
- `Rally` is the destination; `LastRally` tracks the visual flag. `Refresh_Rally`, selection hooks, event redraw, and `Limbo` collectively manage its lifecycle. Preserve owner checks and selection-only visibility.
- Use cell flags (`IsFlagged`, `Flag_Place`/`Flag_Remove`, `ANIM_FLAG`/`FLAGFLY`) for the marker. A `CC_Draw_Line` marker became a glowing laser through the Remastered intercept. Placement checks must ignore rally markers without generally disabling real flag/occupancy rules.
- `dllinterface.cpp` dynamic-map export must not reveal opponent flags. Test classic and Remastered graphics separately if touching that code.
- Service queues are separate from production queues: `DockQueue` stores `TARGET` handles on the building. `Dock_Advance` prunes stale heads; the waiting unit's `RADIO_DOCKING` poll performs promotion. Preserve cleanup on detach, cancellation, departure, and building limbo.
- Helipad production cannot trust `In_Radio_Contact()` alone: check active, non-limbo aircraft physically landed on the dock cell before spawning a new helicopter (`c091c01`).

## Standalone UI and balance

- Guard production queue declarations/state, constructor initialization, busy-factory handling, advancement after abandonment, and sidebar click behavior with `REMASTER_BUILD` boundaries. GlyphX has its own queue system. Standalone queues are **units only**, with five total per category: one active plus four waiting. Buildings retain normal hold/resume/placement without queueing. Keep the existing queue-array storage size and reserved building/special slots; changing queue policy does not require changing serialized class layout.
- SDL2 screen shake is presentation-only (`Request_Video_Shake`); do not reintroduce a `Frame_Limiter` loop inside an explosion. The old snapshot/blit loop blocks simulation. RA and non-SDL2 backends still have their original paths unless deliberately changed.
- Double-click matches `Techno_Type_Class()` and filters ownership, selectable/discovered state, cloak, and tactical viewport. Buildings are excluded; Shift adds. An already-selected target can report `ACTION_NONE`, so do not narrow that release gate. Keep helper/state/handler standalone-only.
- Nuke changes in `anim.cpp` separate campaign from non-campaign damage. The Ion Cannon area-damage change applies to all modes. Do not describe all superweapon changes as skirmish-only; see `docs/komkommer.md` for current values.

Build target: `KomkommerTD` (standalone) or `TiberianDawn` (DLL). Test fresh games; building/house layout changes invalidate assumptions about older saves.
