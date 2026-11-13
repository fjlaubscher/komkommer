# Working on Komkommer

Gameplay/QoL fork of Vanilla Conquer, not a vanilla-balance port. Build and run: `README.md`. Player-visible behavior: `docs/komkommer.md`.

These guides keep traps the code does not state. Leave procedures, file tours, and commit indexes out.

## Layout

- `tiberiandawn/` and `redalert/` are separate engines. Check the counterpart when a change applies to both. Engine traps are in each directory's `AGENTS.md`.
- `common/` is platform input, video, audio, paths, and shared utilities. `resources/remaster_mods/` is source mod content. `build*/` is generated. `common/AGENTS.md` and `resources/AGENTS.md` hold their traps too.

## Sharing code

Share the walk, not the bytes. When one line differs, put the walk in a small header and pass a function. `common` is compiled without `REMASTER_BUILD`, so a standalone-only helper is included from the game file inside that guard.

A saved object is a memory dump, padding included. `SAVEGAME_VERSION` is a sum of `sizeof` the saved classes. Adding or removing a field fails the load. Reordering two fields of the same size does not, and the bytes land in the wrong members. `HouseClass` and `BuildingClass` are in that sum. Initialize new state and test a fresh game.

`EventClass` is the multiplayer packet. `Frame` is 27 bits in TD and 26 in RA. `RALLY` is 27 in TD and 33 in RA, because `FIXIT_VERSION_3` inserts draw events ahead of it. Keep `EventLength` and `EventNames` with the enum.

Do not unify these. They are different bytes, type ids, or host contracts:

- Saved classes (`house`, `building`, `techno`, `unit`, `infantry`, `aircraft`). RA's `AbstractClass` has `Height`. TD aircraft have `Altitude`. RA has vessels. TD's `TechnoClass` inherits `CrewClass`.
- `defines.h` type enums. RA adds vessels.
- `*data.cpp` stat tables. RA also has `vdata.cpp`.
- `audio.cpp` is the name table. Playback is `soundio*` in `common/`.
- `scenario.cpp`. RA's `Read_Scenario_INI` creates houses, teams, and triggers. TD's does not.
- `saveload.cpp`. TD writes through `FileClass`. RA writes through a pipe. The version sum lists different classes.
- `dllinterface.h`. `DLL_LAYER_COUNT` is 3 in TD and 4 in RA. TD's map size also depends on `MEGAMAPS`.
- `sidebarglyphx`. `MAX_BUILDABLES` is 30 in TD and 75 in RA. Both write `sizeof(*this)`.
- IPX. TD magic is `0x1234`, product `0xaa01`. RA magic is `0x1235`, products `0xaa01` and `0xaa00`.

## Standalone versus Remastered

- Standalone double-click selection and production queues stay behind `#ifndef REMASTER_BUILD`, including their fields and call sites. Do not port that UI into the Remastered host.
- `REMASTER_BUILD` is compile-time. `RunningAsDLL` and `GAME_GLYPHX_MULTIPLAYER` do not exclude class state.
- Remastered DLLs are x86 only. Never reuse a CMake cache for another architecture or compiler.
- TD `MEGAMAPS` is standalone-only. Remastered leaves it off because foundation row offsets were wrong.

## Simulation

- Player commands go through `OutList` / `EventClass`. Do not drive the simulation from the wall clock.
- Campaign is `GameToPlay == GAME_NORMAL` in TD and `Session.Type == GAME_NORMAL` in RA. A custom single-player map can still be the campaign path.
- Do not keep a raw pointer to a pooled object. It can leave the heap.
- New `HouseClass` or `BuildingClass` fields break saves. Test a fresh game.
- Match nearby C++11 and `.clang-format`. Do not reformat legacy non-UTF-8 source.
- Player-visible changes go in `docs/komkommer.md`.

## Build

Configure from `README.md`. The `release` preset tree defaults to `RelWithDebInfo`. `Debug` is another config in that tree.

- Exit `0xc0000135` is a missing SDL2 or OpenAL DLL, not a failed assertion.
- A compile-time boundary or shared header needs both standalone games and both x86 Remastered DLLs. A standalone build misses symbols guarded by `REMASTER_BUILD`.
- Remastered deploy is `.agents/skills/remaster-mod/SKILL.md`. Do not deploy, or overwrite an install, just to see that it compiles. Restart the host after a DLL update.
- For a local run, put `[Paths] DataPath=...` beside the build executable. Do not hard-code a data path into source.
- CTest does not cover gameplay. Say what built and which mode.
- `cmake --build <tree> --target format` reformats sources; it only exists when clang-format is found. Pin clang-format to 14.0.x — the build warns outside 14.0.0–14.0.6.
- CI builds with `-Werror=write-strings` (Linux/macOS) and `/Zc:strictStrings` (MSVC), so assigning a string literal to a non-`const` `char*` fails in CI even though a local build without those flags passes. Reproduce locally by setting the `VC_CXX_FLAGS` env var before configuring (it applies to GCC/Clang too, despite the name).

When a change can affect them, check click and box selection (off-screen, enemy, cloaked, aircraft), production hold/resume/cancel and placement, rally set/move/deselect/destroy, several dock customers including a cancelled one and an occupied helipad, and campaign separately from skirmish.

## Packaging

- CI releases only on `v*` tags. Do not tag unless asked.
- Do not commit credentials, game data, binaries, PDBs, local configs, or packaging archives. Leave unfamiliar untracked files alone.
- `scripts/package-macos.sh` deletes and recreates its output directory. Do not point it at a directory that holds other work.
