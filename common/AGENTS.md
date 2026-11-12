# Shared engine/platform code

Changes here can affect both games. Read the root `AGENTS.md`, including Sharing code between the games, for what belongs in this directory and what stays in each game. Compile and test both TD and RA for a shared input or utility change.

## Double-click input pipeline

The standalone path is a chain, not just a selection loop:

1. `wwkeyboard_sdl2.cpp`: synthesize consecutive left clicks using elapsed milliseconds and cursor distance. Homebrew's SDL2 compatibility layer does not reliably forward SDL3's click count, so do not rely solely on `event.button.clicks`.
2. `wwkeyboard.h/.cpp`: `Put_Mouse_Message` transports the double-click flag with the buffered mouse event; `Has_Native_Double_Click()` tells the display layer whether the backend supplies detection.
3. `gadget.cpp/.h`: `LEFTDBLCLICK` must survive dispatch and `GadgetClass::Clicked_On`'s flag mask when a gadget accepts left-button input. Silently masking it out makes the game see ordinary single clicks.
4. Each game's `display.cpp`: forward the flag into `Mouse_Left_Release`, then select matching types. The on-screen walk is `Select_Matching_On_Screen` in `selectscan.h`; each game passes the filter and the screen point (TD aircraft subtract `Altitude`, RA aircraft use `Center_Coord`). Include that header only outside `REMASTER_BUILD`. GlyphX owns the Remastered client's selection UI. Box select stays in `Select_These`.

SDL2 detection currently uses a `<500 ms` press interval and at most 32 pixels per axis, calculated in SDL window coordinates. Mouse positions passed to the game still honor raw-input/gamepad handling and video scale. Keep those coordinate systems separate.

The display-layer fallback uses `WinTickCount.Time()`, not the simulation `Frame` counter. Do not restore the superseded frame-counter detector from `0fd1706`. See `docs/double-click-select-all.md` and commit `91d579a`.

Regression checks: native Windows/backend behavior where applicable, SDL2 (especially macOS/sdl2-compat), single-click, drag/box select, Shift, right-click cancel, and clicks on an already-selected object.

## Paths and data

- `skirmishroster.h` shares lobby-only slot/colour/team logic between TD (six slots) and RA (eight). `skirmishdrop.h` is a small template instantiated with each game's own native `DropListClass`. Include the dropdown helper only in standalone game code; the common library itself has no `REMASTER_BUILD` definition. Preferences, map parsing, country enums, palettes, rule limits, and startup/recording integration stay in each engine's `skirmish*` files. These helpers are not saved engine-class state.

- `paths.cpp` resolves config/data paths; `paths_win.cpp` and `paths_posix.cpp` provide platform defaults. A config beside the executable can redirect `DataPath` and makes that directory the default `UserPath` unless explicitly overridden.
- The retained `Vanilla-Conquer` / `vanillatd` / `vanillara` user-data names are not necessarily a missed rebrand. Changing them can strand existing settings and saves.
- Use the engine file abstraction when loading game/mod assets. A loose-file check alone misses files available inside MIX archives.

## Portability

Keep shared APIs compatible with C++11 and all configured backends, including SDL1 where present. Handle pointer/integer conversions with pointer-width types (`uintptr_t`), not `unsigned int`. Host-facing layouts must not change accidentally when fixing compiler errors.

Check `common/CMakeLists.txt` before adding build guards here: `common` is shared and is not compiled with `REMASTER_BUILD`; `commonr` and `commonv` have separate definitions. Do not conditionally change a shared class layout in only its consumers. Keep the standalone/GlyphX selection boundary in the game targets, where that definition is available.
