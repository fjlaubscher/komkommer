# Shared engine/platform code

A change here affects both games. Compile both. What stays in each game is in the root guide.

`common` is not compiled with `REMASTER_BUILD`. `commonr` and `commonv` are. Do not change a shared class layout in only one of them, and do not put a standalone-only field here.

## Double-click

The pipeline and the regression list are in `docs/double-click-select-all.md`.

- SDL2-compat does not reliably forward SDL3's click count. Do not trust `event.button.clicks` alone.
- `LEFTDBLCLICK` is dropped if `GadgetClass::Clicked_On` masks it out. The game then sees an ordinary click.
- The display fallback times with `WinTickCount`, not the simulation `Frame`. Do not restore the old frame-counter detector.
- The 500 ms / 32 px test uses SDL window coordinates. Positions handed to the game are in game coordinates.

## Paths

- `skirmishroster.h` and `skirmishdrop.h` are lobby-only and are not saved engine state. Include the dropdown template only from standalone game code. Slot counts, countries, and startup stay in each game.
- A config beside the executable sets `DataPath` and, unless overridden, `UserPath`.
- `Vanilla-Conquer`, `vanillatd`, and `vanillara` are the existing user-data directories. Renaming them strands settings and saves.
- Load assets through the engine file classes. A loose-file check misses MIX archives.

## Portability

C++11, including the SDL1 backends. Pointer-sized values use `uintptr_t`, not `unsigned int`.
