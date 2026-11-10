# Working on Komkommer

Komkommer is a gameplay/QoL fork of Vanilla Conquer, not a strict vanilla-balance project. Read `README.md` for build/run instructions and `docs/komkommer.md` for intended gameplay differences.

## Layout and scope

- `tiberiandawn/` and `redalert/` are separate engines with similar, but not interchangeable, implementations. Check the counterpart when changing a feature shared by both games. How to share a piece is under Sharing code between the games.
- `common/` contains platform input/video/audio, file/path handling, and shared utilities.
- `resources/remaster_mods/` contains source mod manifests/assets; `build*/` is generated output.
- `cmake/`, `CMakePresets.json`, `.github/workflows/`, and `scripts/` describe builds and packaging.
- Directory-local `AGENTS.md` files add engine-specific guidance. Keep general rules here and implementation details close to their code.

## Sharing code between the games

`tiberiandawn/` and `redalert/` stay two engines. Share a piece when the idea is the same in both, and leave the game-specific part in each game. The input path, file paths, and the `common` / `commonv` / `commonr` build split are in `common/AGENTS.md`.

- Platform code goes in `common/`: keyboard and mouse (`wwkeyboard*`, `wwmouse`, `gadget`), video (`video_*`), files and paths (`file.cpp`, `paths.cpp`), sound (`soundio*`), and settings (`settings.cpp`).
- A field that gets saved stays on that game's own class. The save copies the object's memory, padding included. TD writes it with `file.Write(Ptr(i), sizeof(T))` in `tiberiandawn/heap.h`. RA writes it with `file.Put(Ptr(i), sizeof(T))` in `redalert/heap.cpp`. On load, each `saveload.cpp` checks `SAVEGAME_VERSION`, a sum of `sizeof` the saved classes. Adding or removing a field changes that sum, so an old save fails the check. Reordering two fields of the same size can leave the sum unchanged: the file still loads, and the bytes land in the wrong members. `HouseClass` and `BuildingClass` are in that sum. Initialize the new state and test a fresh game.
- `EventClass` stays in each game (`event.h`, `event.cpp`). It is the command sent to the other players, so both computers have to use the same numbers. `Frame` is 27 bits in `tiberiandawn/event.h` and 26 bits in `redalert/event.h`. `RALLY` is 27 in TD. In RA it is 33: `FIXIT_VERSION_3` is set in `redalert/defines.h`, `function.h` includes that before `event.h`, and `PROPOSE_DRAW` and `RETRACT_DRAW` sit ahead of `RALLY`. Changing a number or a bit width makes the other computer run a different command, which desyncs multiplayer. Keep `EventLength` and `EventNames` lined up with the enum.
- When the walk is the same and one line differs, put the walk in a small header and pass a function for that line. `common/selectscan.h` walks the on-screen objects; each game passes who counts and where they are drawn. TD aircraft subtract `Altitude`. RA aircraft pass `Center_Coord()`. RA stores altitude in `Height` on `AbstractClass`. `common/productionqueue.h` is one queued build: a one-byte type and an id, 8 bytes with the id at byte 4. The category list stays in each game. TD's list has a special slot. RA's list has a vessel slot. Adding and cancelling stay on each `HouseClass`, because RA also queues vessels and the two games check the house differently. A dock queue is the same idea. `Is_Dock_Queueable()` is false for an airstrip in both games, and RA fixed-wing rearming uses its own landing path, so that test stays in the game.
- A helper that exists only for the standalone build is included from the game file inside `#ifndef REMASTER_BUILD`. `display.cpp` includes `selectscan.h` that way, and `house.h` includes `productionqueue.h` that way. The `common` library is compiled without `REMASTER_BUILD`. See `common/AGENTS.md`.

Leave these files in each game. They look similar, and they are different bytes, type lists, or host contracts:

- `HouseClass`, `BuildingClass`, `TechnoClass`, and the unit, infantry, and aircraft classes (`house.h`, `building.h`, `techno.h`, `unit.h`, `infantry.h`, `aircraft.h`). The save copies the whole object. RA's `AbstractClass` has `Height`, TD aircraft have `Altitude`, RA has vessels, and TD's `TechnoClass` also inherits `CrewClass` while RA stores `Crew` as a member.
- `defines.h`. The building, infantry, unit, and house enums are different lists, and RA adds a vessel enum. Those numbers are the type ids used through the rest of the game.
- The `*data.cpp` stat tables, such as `udata.cpp`, `bdata.cpp`, `idata.cpp`, and `aadata.cpp`. Each file holds that game's numbers for its types. RA also has `vdata.cpp` for vessels.
- `audio.cpp`. This is the sound-effect name table, `SoundEffectName`. Shared playback is `soundio*` in `common/`.
- `scenario.cpp`. Each game loads its own mission. RA's `Read_Scenario_INI` creates houses, team types, and triggers from the scenario INI. TD's `scenario.cpp` does not.
- `saveload.cpp`. TD writes through `FileClass`. RA writes through a pipe. The version sum lists different classes: RA includes `VesselClass`, the trigger classes, `ScenarioClass`, and `ChronalVortexClass`. TD includes the briefing text and the waypoints.
- `EventClass` in `event.h` and `event.cpp`. The enum values and the bit fields are the multiplayer packet.
- `dllinterface.h`. These structs are what the Remastered host reads from the DLL. `DLL_LAYER_COUNT` is 3 in TD and 4 in RA, and TD's map size also depends on `MEGAMAPS`.
- `sidebarglyphx.h` and `sidebarglyphx.cpp`. This is the Remastered sidebar. `MAX_BUILDABLES` is 30 in TD and 75 in RA. TD saves it through `FileClass`, RA through a pipe, and both write `sizeof(*this)`.
- The IPX stack in each game: `ipxconn.*`, `ipxgconn.*`, and `ipxmgr.*`. TD's global magic number is `0x1234` and its product id is `0xaa01`. RA's magic number is `0x1235`, with product ids `0xaa01` and `0xaa00`. Address types are already shared in `common/ipx.h` and `common/ipxaddr.*`.

## Standalone versus Remastered

- **Do not duplicate GlyphX's native UI behavior.** Double-click selection and production queueing added for standalone belong behind `#ifndef REMASTER_BUILD`, including state fields, initialization, and call sites. Keep the original Remastered factory rejection/restart behavior.
- Existing intentional Remastered mod features (rally points, service queues, unit/balance changes) are not an invitation to port standalone input/sidebar logic into the host. Change those features only within the requested scope.
- `REMASTER_BUILD` is a compile-time boundary; `RunningAsDLL` and `GAME_GLYPHX_MULTIPLAYER` are runtime concepts, not substitutes for excluding standalone-only class state.
- Remastered loads **x86 DLLs only**. Standalone can be x64. Never repurpose an existing CMake cache for a different architecture/compiler.
- TD standalone enables `MEGAMAPS`; TD Remastered intentionally does not, because placement-footprint row offsets were wrong with it enabled. Do not re-enable it without fixing and testing that boundary.

## Engine-change rules

- Keep player-issued simulation changes on the existing `OutList`/`EventClass` path. Selection/display feedback and actual simulation commands are different concerns. Maintain deterministic behavior for multiplayer; do not use wall-clock timing to drive simulation state.
- Audit campaign versus skirmish/MP gates: TD uses `GameToPlay`, RA uses `Session.Type`; `GAME_NORMAL` identifies the campaign path. A single-player custom sandbox is not proof of skirmish behavior.
- Preserve pooled-object lifecycle and handle conventions (`TARGET`, `TargetClass`, `As_Target`, `IsActive`, `IsInLimbo`, `Detach`, `Limbo`). Do not retain a raw pointer to an object that can leave a heap or be destroyed.
- Class layout matters: a save is a memory dump of the engine objects (see Sharing code between the games). New `HouseClass`/`BuildingClass` fields can break save compatibility; initialize new state and test fresh games. Do not claim old saves work without testing.
- Match nearby C++11 style and the existing `.clang-format`. Avoid unrelated reformatting: some old source contains legacy/non-UTF-8 text.
- Update `docs/komkommer.md` when intentionally changing player-visible behavior. Keep verified implementation notes distinct from plans and untested assumptions.

## Build and validation

Standard presets use Ninja Multi-Config. `release` defaults to `RelWithDebInfo`, not `Release`; `Debug` is a separate configuration in the same tree.

```sh
cmake --build build/release --config Debug --target KomkommerTD KomkommerRA
cmake --workflow --preset testing
```

Initialize the correct MSVC developer environment on Windows before building. The standalone dependencies are SDL2 and OpenAL; both link libraries and runtime DLLs must match the target architecture. See `README.md` for configure commands.

Windows CTest runs also need SDL2/OpenAL runtime DLLs on `PATH` or beside the test executables. Exit `0xc0000135` means a loader dependency is missing, not necessarily a failing test assertion; fix the test environment and rerun before diagnosing engine code.

When editing compile-time boundaries/shared headers, compile **both standalone games and both x86 Remastered DLLs**. A successful standalone build cannot catch missing symbols or layout mistakes hidden by `REMASTER_BUILD`.

For Remastered deployment, read `.agents/skills/remaster-mod/SKILL.md` and use its helper. Do not deploy, launch a game, or overwrite installed game files merely to check compilation unless requested. Restart the host after a DLL update.

For a standalone dev test, prefer a config next to the build executable with `[Paths] DataPath=...` rather than replacing the installed binary. A local config also isolates writable settings/saves by default. Discover the user's data directory; do not bake a developer's absolute path into source or general docs.

The CTest suite covers low-level utilities, not gameplay. Report what actually built/tested and which mode. Useful manual checks:

- Single/Shift/double-click and box selection; off-screen, enemy, cloaked, and aircraft cases.
- Production completion, hold/resume/cancel, queue cap, and building placement.
- Rally flag set/move/deselect/destruction, placement over the marker, opponent visibility, and classic/HD rendering when relevant.
- Multiple dock customers, cancelled/destroyed customers, occupied helipads, and repairs returning to rally points.
- Campaign and skirmish/MP separately for mode-gated changes.

## Packaging and repository hygiene

- CI workflows trigger only on `v*` tag pushes. An ordinary push does not create a development release. Do not create/push a release tag without being asked.
- Keep credentials, game data, binaries, PDBs, local configs, and packaging archives out of commits. Leave unfamiliar untracked files alone.
- macOS packaging is in `scripts/package-macos.sh`; signing for CI is in `.github/scripts/sign-macos-app.sh`. The local packaging script recreates its output directory. Never point it at a directory containing user work.
- Capture durable lessons in these guides, not session logs. Cite useful commits, but verify the current code before following old commit messages or comments.

## Useful history

- `0854969`, `0896eeb`, `c091c01`: rally rendering, selection/redraw, ownership filtering, helipad occupancy.
- `88acdf1`: radio-based service queues, TD placement proximity, RA repair targeting.
- `4308427`, `846d58c`: superweapon balance and campaign/MP boundaries.
- `75c5546`, `f052d7b`, `5bc064c`: commando limits, Saboteur type/voice/name/cameo paths, TD Nod build/AI access.
- `fcc9076`: aircraft sight defaults are insufficient without fixing terrain-reveal call sites.
- `91d579a`, `14d8c6d`: current double-click input path; the older `0fd1706` frame-counter selection implementation was superseded, but its standalone production queues were retained.
- `d8466c3`: macOS packaging/signing and the SDL3 runtime dependency missed by static dylib discovery.
