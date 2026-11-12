# Komkommer

A personal Command & Conquer engine fork, based on [Vanilla Conquer](https://github.com/TheAssemblyArmada/Vanilla-Conquer), for **Tiberian Dawn** and **Red Alert**. Komkommer keeps the classic games' look and feel while adding quality-of-life improvements and deliberate gameplay changes.

There are two ways to play:

| Build | What it runs |
| --- | --- |
| **Standalone** | `komkommertd` / `komkommerra`, using classic game data on Windows, Linux, or macOS |
| **Remastered mod** | `TiberianDawn.dll` / `RedAlert.dll`, loaded by the Command & Conquer Remastered Collection on Windows |

Game assets are not included. The standalone engine and the Remastered client have different input, sidebar, and rendering paths; standalone UI improvements do not replace GlyphX's native implementations.

## What's different?

- **Standalone double-click selection:** select all on-screen units of the same type; hold Shift to add them to the selection.
- **Standalone unit production queues:** up to five units per category, including the unit being built. Buildings remain one-at-a-time.
- **Standalone skirmish setup:** size-sorted maps with player counts, country/faction/colour/team dropdowns, configurable AI rosters, and persistent match settings in both games.
- **Rally points:** send newly produced and repaired units to a destination, with a player-coloured flag visible while the building is selected.
- **Service queues:** vehicles and aircraft can wait for repair bays, refineries, and helipads. RA airstrip rearming remains separate.
- **Aircraft scouting:** aircraft sight defaults to three cells, with terrain-reveal fixes.
- **TD:** more forgiving base placement, Nod Guard Tower/Repair Bay access, a one-per-house commando limit, and revised Ion Cannon/nuke damage.
- **RA:** a Soviet **Saboteur** commando for skirmish/multiplayer, commando limits, repair-targeting fixes, and revised nuke damage.

See [gameplay changes](docs/komkommer.md) for details and mode-specific exceptions. This is a mod, not an unchanged recreation of vanilla balance.

## Play standalone

Use classic **C&C Gold** data for TD or classic **Red Alert** data for RA. The official RA demo also works with the engine. Other editions/repackages can have data differences; reproduce asset-related bugs with classic retail/freeware data where possible.

On Windows, put the matching executable beside the classic game data, together with architecture-matched `SDL2.dll` and `OpenAL32.dll`. Do not overwrite the original game executable.

Alternatively, keep the build separate and put a `conquer.ini` (TD) or `redalert.ini` (RA) beside the executable:

```ini
[Paths]
DataPath=D:\Games\VANILLATD

[Intro]
PlayIntro=no
```

Change `DataPath` to your game-data directory. A config beside the executable also makes that directory the default location for writable settings/saves; set `UserPath` under `[Paths]` to override it. Without a local config, Windows user data normally lives in `%APPDATA%\Vanilla-Conquer\vanillatd` or `vanillara` (the upstream directory names are retained).

**Start a fresh game when testing engine changes.** Added state in `HouseClass` and `BuildingClass` can make saves incompatible between builds. All players in a multiplayer game should use the same build and gameplay data.

## Build standalone

Requirements: **CMake 3.25+**, a C++11 compiler, **Ninja** for the standard presets, **SDL2**, and **OpenAL**. ImageMagick is optional for generating application icons.

### Windows (MSVC)

Install Visual Studio C++ Build Tools and a Windows SDK. In an **x64 Native Tools** terminal, point CMake at the SDL2 and OpenAL development packages:

```powershell
cmake --preset release -DSDL2_ROOT_DIR="C:/deps/SDL2" -DOPENAL_ROOT="C:/deps/openal-soft"
cmake --build --preset release
```

This produces both executables under `build/release/RelWithDebInfo/`. Dependency libraries and runtime DLLs must match the compiler architecture.

For a development build of just TD:

```powershell
cmake --build build/release --config Debug --target KomkommerTD
```

The binary and symbols are `build/release/Debug/komkommertd.exe` and `.pdb`. Copy the runtime DLLs into that directory and configure its data path before launching. Use target `KomkommerRA` for Red Alert.

The preset name `release` selects a build tree, not a compiler configuration: its default build configuration is **RelWithDebInfo**. **Debug** is unoptimized. Do not reuse a CMake build tree across different compilers or x86/x64 architectures.

### Linux

For Debian/Ubuntu:

```sh
sudo apt install build-essential cmake ninja-build libsdl2-dev libopenal-dev
cmake --preset release
cmake --build --preset release
```

For Fedora, install `gcc-c++ cmake ninja-build SDL2-devel openal-soft-devel` instead. The executables are in `build/release/RelWithDebInfo/`. To keep game data separate, set `[Paths] DataPath` in `~/.config/vanilla-conquer/vanillatd/conquer.ini` or `vanillara/redalert.ini` (or under `$XDG_CONFIG_HOME` when set).

### macOS

Install the Xcode command-line tools and Homebrew dependencies:

```sh
brew install cmake ninja sdl2 openal-soft
cmake --preset release -DCMAKE_PREFIX_PATH="$(brew --prefix);$(brew --prefix openal-soft)"
cmake --build --preset release
```

The app bundles are `build/release/RelWithDebInfo/komkommertd.app` and `komkommerra.app`; game assets are still separate. Set `[Paths] DataPath` in `~/Library/Application Support/Vanilla-Conquer/vanillatd/conquer.ini` for TD or `vanillara/redalert.ini` for RA, using an absolute path to the classic game data.

For portable distribution of both apps:

```sh
brew install dylibbundler
bash scripts/package-macos.sh
```

The script bundles libraries, signs the staged apps, and writes ZIPs to `build/release/macos-dist/`. It also handles the SDL3 runtime loaded dynamically by Homebrew's `sdl2-compat`. A Developer ID identity and App Store Connect API credentials enable notarization; without them it falls back to ad-hoc signing or skips notarization as applicable. Keep credentials in the ignored `scripts/macos-notary.env`, never in source control. The output directory is recreated on each run.

### Other presets and tests

See [CMakePresets.json](CMakePresets.json) for `internal` (editors/tools), `nonet`, `testing`, and `remaster`. Local overrides can go in the ignored `CMakeUserPresets.json`; start from [the example](resources/CMakeUserPresets.json.example).

```sh
cmake --workflow --preset testing
```

On Windows, configure the `testing` preset with the same SDL2/OpenAL paths first. Make their runtime DLLs available on `PATH` or beside the test executables in `build/testing/RelWithDebInfo/`; a successful link does not make those DLLs discoverable at runtime.

The tests cover low-level engine utilities, not complete gameplay. Selection, production, docking, and rendering changes still need in-game testing in the affected modes.

## Build and install Remastered mods

Remastered requires **32-bit Windows DLLs**, even on 64-bit Windows. SDL2/OpenAL are not required for DLL-only builds. GlyphX already owns double-click selection and production queue UI; Komkommer's standalone implementations are excluded from these builds.

From an **x86 Native Tools** terminal, using a separate build tree:

```powershell
cmake --preset remaster
cmake --build --preset remaster
```

The build stages complete mod folders in `build/remaster/komkommer_TD` and `build/remaster/komkommer_RA`. Launch each Remastered game once to create its mod directories, then copy the corresponding folder into:

```text
<Documents>/CnCRemastered/Mods/Tiberian_Dawn/komkommer_TD/
<Documents>/CnCRemastered/Mods/Red_Alert/komkommer_RA/
```

Some installations use `<Documents>/CnCRemastered/CnCRemastered/Mods/` instead. Keep `ccmod.json` at the mod root and the DLL in `Data/`; copy the whole folder, including additional assets/XML. Enable **Komkommer TD** or **Komkommer RA** in the in-game mod list and restart the game after updating a DLL.

The repository also includes a Windows build-and-deploy helper that discovers the Visual Studio toolchain and Documents path, validates DLL architecture, and copies the mod and symbols:

```powershell
powershell -NoProfile -ExecutionPolicy Bypass -File .agents/skills/remaster-mod/scripts/deploy.ps1 -Game Both
```

Use `-Game TD` or `-Game RA` for one game, and `-Config Debug` for an unoptimized DLL. Close the game before deploying; it locks loaded DLLs.

## Documentation and releases

- [Gameplay changes](docs/komkommer.md)
- [Standalone double-click implementation](docs/double-click-select-all.md)
- [Local Remastered sandbox notes](docs/komkommer-RA-1p-test.md) — machine-specific examples, not bundled maps
- [Contributor/agent guide](AGENTS.md), with more specific guides in the source directories
- [Komkommer releases](https://github.com/fjlaubscher/komkommer/releases) and [issues](https://github.com/fjlaubscher/komkommer/issues)

CI builds and publishes releases on **`v*` tag pushes**, not ordinary branch pushes or pull requests. Upstream Vanilla Conquer binaries do not contain Komkommer's changes.

## Credits and license

Built on the work of Westwood, Electronic Arts' [released source](https://github.com/electronicarts/CnC_Remastered_Collection), and the Vanilla Conquer contributors. Distributed under the **GPLv3**, with the additional terms described in [License.txt](License.txt). Game assets remain the property of their respective owners.
