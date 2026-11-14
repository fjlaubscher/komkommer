---
name: debug-loop
description: >-
  Build the standalone Tiberian Dawn and Red Alert debug binaries, point them at the
  classic game data and the player's own video/sound settings, and run them.
  Use when the user wants to try an engine change in the standalone game, asks for
  a debug build, or wants the debug binaries to use their data and their settings.
---

# Standalone debug loop

Build the standalone debug binaries and stage them so the game starts with the classic
data and the settings the player already likes.

The script is `scripts/debug-run.ps1` beside this file. From the repo root:

```powershell
powershell -NoProfile -ExecutionPolicy Bypass -File .agents/skills/debug-loop/scripts/debug-run.ps1 -Game TD
```

## Which game

- Tiberian Dawn, TD, or edits under `tiberiandawn/` → `-Game TD`
- Red Alert, RA, or edits under `redalert/` → `-Game RA`
- Both, shared `common/` code, or the user did not say → `-Game Both`

## What the script does

1. Builds `KomkommerTD` / `KomkommerRA` in the `Debug` configuration of `build/release`.
2. Writes the config next to each debug binary. It holds the classic game data path
   and a copy of the release settings, so the debug run looks and sounds like the
   played release.
3. Removes `savegame.*` and `hallfame.dat` from the debug folder, so the game starts
   a new game instead of loading a save from an older build.
4. Prints the binary, config, and data paths.

The debug binaries land in `build/release/Debug/`. The release folder in
`C:\BoomerGames\KOMKOMMER` is only read. The debug build keeps its own settings and
saves next to the binary, so a debug run never writes into the release folder.

The config keeps the release values for video, sound, mouse, hotkeys, and the
skirmish lobby. Only the `[Paths]` section is rewritten, so the debug run reads the
classic data and stores its saves beside the binary.

## Flags

| Flag | Effect |
| --- | --- |
| `-Game TD\|RA\|Both` | Which game to build and stage. |
| `-Config Debug\|RelWithDebInfo` | Build configuration. `Debug` is the default. |
| `-BuildTree <path>` | CMake tree to use. `build/release` by default. |
| `-Clean` | Remove the debug binary and config of the selected games before building. |
| `-Rebuild` | Recompile everything in the tree before building. |
| `-KeepSaves` | Keep `savegame.*` and `hallfame.dat` in the debug folder. |
| `-KeepSettings` | Keep the debug config that is already there instead of importing the release settings. |
| `-Launch` | Start the game after the config is staged. |
| `-TdDataPath` / `-RaDataPath` | Game data folder. `D:\Games\VANILLATD` and `D:\Games\VANILLARA` by default. |
| `-SettingsRoot <path>` | Folder that holds the played release settings. `C:\BoomerGames\KOMKOMMER` by default. |

## After the script

Report the binary path, the config path, and the data path from the script output.

Do not launch the game unless the user asks for it or passes `-Launch`. The game needs
the SDL2 and OpenAL runtime DLLs beside the binary; the script copies them when they
are missing.

A fresh game matters after engine changes. `HouseClass` and `BuildingClass` layout
changes break saves, so the script drops the old saves by default.

CTest does not cover gameplay. Say what was built and that the change still needs an
in-game check.
