# Working on Komkommer

Komkommer is a gameplay/QoL fork of Vanilla Conquer, not a strict vanilla-balance project. Read `README.md` for build/run instructions and `docs/komkommer.md` for intended gameplay differences.

## Layout and scope

- `tiberiandawn/` and `redalert/` are separate engines with similar, but not interchangeable, implementations. Check the counterpart when changing a feature shared by both games.
- `common/` contains platform input/video/audio, file/path handling, and shared utilities.
- `resources/remaster_mods/` contains source mod manifests/assets; `build*/` is generated output.
- `cmake/`, `CMakePresets.json`, `.github/workflows/`, and `scripts/` describe builds and packaging.
- Directory-local `AGENTS.md` files add engine-specific guidance. Keep general rules here and implementation details close to their code.

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
- Class layout matters: saves serialize engine objects. New `HouseClass`/`BuildingClass` fields can break save compatibility; initialize new state and test fresh games. Do not claim old saves work without testing.
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
