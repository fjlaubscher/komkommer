# Tiberian Dawn engine

Read the root guide first. TD constants and fields are not RA's.

## Where to look

| Concern | Entry points |
| --- | --- |
| Selection and placement | `display.cpp/.h` |
| Production and base AI | `house.cpp/.h`, `sidebar.cpp`, `factory.cpp`, `bdata.cpp` |
| Rally, docks, exits | `building.cpp/.h`, `event.cpp/.h`, `drive.cpp`, `foot.cpp`, `techno.cpp`, `unit.cpp` |
| Aircraft sight | `aadata.cpp`, `aircraft.cpp` |
| Nuke and Ion Cannon | `anim.cpp` |
| Remastered UI | `dllinterface.cpp/.h`, `sidebarglyphx.*` |
| Standalone skirmish | `skirmish.cpp`, `skirmishsettings.h`, `scenarioini.cpp` |

## Traps

- Campaign is `GameToPlay == GAME_NORMAL`, not RA's `Session.Type`.
- Aircraft altitude is `Altitude`. A ground-only double-click sweep misses fliers. Sight reveals use `Class->SightRange`; the `aadata.cpp` default alone does not fix the call sites.
- `Can_Build` checks the one-Rambo limit in more than one path. The sidebar does not enforce it.
- Nod Guard Tower and Repair Bay need both `bdata.cpp` and the base AI in `house.cpp`. The AI repair bay accepts a War Factory or an Airstrip.
- Placement allows a one-cell gap from a friendly building and still requires a legal radar, foundation, and proximity check.
- `MEGAMAPS` is standalone-only. Remastered leaves it off because foundation row offsets were wrong. Do not flip it as cleanup.
- `Can_Rally` is a whitelist, not "anything that produces." `Rally` is the order; `LastRally` is the flag. A drawn line renders as a laser in Remastered. Ignore rally markers in placement without turning off real occupancy. Dynamic-map export must hide opponent flags.
- `Dock_Advance` only drops stale heads; the waiting unit's `RADIO_DOCKING` poll takes the dock. Clear the queue on detach, cancel, departure, and building limbo.
- Harvester `ArchiveTarget` is the last ore cell, not the dock (`TiberiumUnloadRefinery`). `FINDHOME` or `RADIO_RUN_AWAY` with an empty hold docks and dumps forever.
- A helipad can be occupied with no radio contact: active, non-limbo, on the ground, on the dock cell.
- Skirmish preferences are `[Skirmish]` in `UserPath/conquer.ini`, not fields on a saved engine class. Keep each map's original index when sorting the lobby list. Do not re-enable `Computer_Paranoid` in standalone skirmish; it replaces the chosen teams. The recording header stores the roster so playback does not read the current lobby.
- Standalone production queues are units only: one active plus four waiting. Buildings are not queued. Do not resize the saved queue arrays to change that policy. `CANCEL_QUEUE` removes the newest waiting match and leaves the active item. Queue code stays outside `REMASTER_BUILD`.
- SDL2 shake is `Request_Video_Shake`. Do not put a `Frame_Limiter` loop inside an explosion.
- Double-click excludes buildings. An already-selected unit can report `ACTION_NONE`. The helper is standalone-only.
- Ion Cannon area damage applies in every mode. Nuke campaign and non-campaign values stay separate. Numbers are in `docs/komkommer.md`.

Build target: `KomkommerTD`, or `TiberianDawn` for the DLL. Building and house layout changes need a fresh game.
