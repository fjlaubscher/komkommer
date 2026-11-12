# Red Alert engine

Read the root guide first. Do not copy TD identifiers or its `Altitude` field.

## Where to look

| Concern | Entry points |
| --- | --- |
| Selection/input | `display.cpp/.h`, shared keyboard/gadget code |
| Production and base AI | `house.cpp/.h`, `sidebar.cpp`, `factory.cpp` |
| Rally, docks, exits | `building.cpp/.h`, `event.cpp/.h`, `drive.cpp`, `foot.cpp`, `techno.cpp`, `unit.cpp` |
| Saboteur | `defines.h`, `idata.cpp`, `infantry.cpp`, `techno.cpp`, `house.cpp`, `dllinterface.cpp` |
| Aircraft sight | `aadata.cpp`, `aircraft.cpp` |
| Nuke | `anim.cpp` (`Do_Atom_Damage`) |

## Traps

- Campaign is `Session.Type == GAME_NORMAL`. A custom single-player map can still take that path.
- Aircraft altitude is `Height`. Reveals use `SightRange`. Stock `Sight=0` becomes 3 after each INI read; a non-zero INI value wins. Do not clamp a landed aircraft to one cell of sight.
- Selection includes vessels, not buildings. Double-click is standalone-only. The second click on an already-selected unit can be `ACTION_NONE`.
- Chronosphere targeting accepts eligible non-friendly units. Keep the type, deploy, and transport exclusions.
- Saboteur reuses the GNRL body and name, so `[GNRL]` INI data can overwrite it; cameo lookup checks `[SABOTEUR]` first. `Read_INI` reapplies the skirmish/MP overrides after the initial table. `Full_Name()` returns the existing `TXT_SABOTEUR` id. Classic cameos try `SBTRICON`, `SABICON`, `GNRLICON`, then Tanya. The Remastered sidebar name falls back to `E7ICON`. One commando per house is `Can_Build`, not the sidebar. Tanya voice, laugh, and death stay Tanya-only.
- Rally markers are cell flags (`ANIM_FLAG`/`FLAGFLY`). A drawn line renders as a laser in Remastered. Markers must not block foundations. Dynamic-map export must hide opponent flags.
- Repair bays, refineries, and helipads queue. The airstrip does not. `Dock_Advance` only drops stale heads; the waiting unit's `RADIO_DOCKING` poll takes the dock. A repair order targets `STRUCT_REPAIR`, not a refinery.
- Harvester `ArchiveTarget` is the last ore cell, not the dock (`TiberiumUnloadRefinery`). `FINDHOME` or `RADIO_RUN_AWAY` with an empty hold docks and dumps forever.
- A helipad can be occupied with no radio contact: active, non-limbo, `Height == 0`, on the dock cell.
- Standalone production queues are absent from the Remastered build. `CANCEL_QUEUE` removes the newest waiting match and leaves the active item. Keep `EventLength` and `EventNames` aligned with the enum.
- SDL2 shake is `Request_Video_Shake`. Keep the blocking `Frame_Limiter` loop only for non-SDL2 backends.
- Game Controls applies speed and scroll on Esc. Sound Controls saves when it closes. Options use `Paths.User_Path()/redalert.ini`, not whichever `redalert.ini` the asset search finds.
- Keep campaign and non-campaign atom damage apart. Current numbers are in `docs/komkommer.md`.

Build target: `KomkommerRA`, or `RedAlert` for the DLL.
