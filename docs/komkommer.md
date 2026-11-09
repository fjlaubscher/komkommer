# Komkommer mod behavior changes (vs stock vanilla)

This document lists gameplay/behavior changes introduced by the **Komkommer** mod compared to stock vanilla **Tiberian Dawn (TD)** and **Red Alert (RA)** behavior in this codebase. It focuses on player-visible/gameplay differences (not rebranding, CI, build/tooling, or internal refactors except where they affect behavior).

## Both games (TD + RA)

### Double-click selects all on-screen units of the same type (standalone)
- **Vanilla:** Double-clicking a unit is treated as two ordinary single clicks (select, then toggle the one unit).
- **Komkommer:** Double-clicking a selectable combat unit selects **every on-screen unit of that exact type** (infantry, vehicles, aircraft, and RA vessels; buildings are excluded).
  - The current selection is replaced, unless **Shift** is held, in which case the matching units are added to it.
  - Matching units that are off-screen, cloaked, or undiscovered are not selected.
  - Double-click detection follows SDL3's standard: two presses within 500 ms and 32 px (`common/wwkeyboard_sdl2.cpp`). Backends without native detection fall back to the `DOUBLE_CLICK_TIME` timer in each game's `display.cpp`.
  - RA also recognizes self-clicks that its unit logic translates into move/no-move or blocked-deploy actions; explicit force-move/force-attack and targeting modes retain priority for those cases.
  - The helper, click state, and handler are excluded from `REMASTER_BUILD`; GlyphX owns the Remastered client's native selection UI.

### Control-group labels (standalone)
- Selected, player-owned units show a tiny gold **3×5-pixel** control-group number below the **bottom-left** of their selection brackets, with a light lower-right shadow (**4×6 pixels overall**, instead of the earlier 5×7 outline). Groups follow the usual keys **1–9, 0**; ungrouped units have no label.
- TD gains the label; RA moves its existing group-number pip below the selection box and retains the **F** suffix for formation members. Infantry, vehicles, aircraft, and RA vessels use the same convention.
- Labels are clipped to the tactical view and hidden on deselection. Remastered rendering is unchanged.

### Aircraft scouting
- Aircraft sight defaults to **3 cells** in both engines. Stock RA `Sight=0` entries are raised to that baseline after rules loading, including helicopters; non-zero INI sight values still override it.
- Terrain-reveal call sites use the type's sight range: TD ground-layer entry/firing, and RA `Look()`/firing. Grounded RA aircraft are no longer clamped to a one-cell sight range.

### Rally points for production buildings
- **Vanilla:** Production buildings do not have a player-settable rally point workflow like this mod’s implementation.
- **Komkommer:** Production buildings support rally points.
  - A left click stores a rally cell through the event queue.
  - Newly produced and fully repaired units move to the rally point.
  - The rally marker is shown as a **player-coloured animated flag** (uses the cell flag/animation system so it renders correctly in both classic and remastered graphics modes).
  - The rally flag is shown **only while the structure is selected**.
  - Rally flags do **not** block structure placement (flagged rally cells are ignored for placement blocking where appropriate).

### Production queueing (standalone binaries)
- **Vanilla:** Clicking a production-sidebar icon while its category factory is busy rejects the click with a "no factory"/scold response.
- **Komkommer (standalone executables only):** Unit production clicks **queue** behind the unit currently being built, up to **5 total units per category**, including the active unit (infantry / vehicle / aircraft / RA naval). Buildings and superweapons are not queued.
  - When the current unit completes and exits, the next queued unit starts automatically.
  - At most **four waiting** requests may sit behind the active unit; further clicks are ignored. Existing save data is not rewritten to discard older excess unit entries; those drain normally. Legacy building-queue entries are no longer advanced.
  - A tiny gold **1–5** label at the **top-left** of each unit cameo shows the total outstanding count of that exact type, including the current unit. A single active unit always shows **1**; other queued types show their own counts. Digits use a fixed 3×5-pixel font plus a one-pixel black outline. Buildings and superweapons have no count. The existing clock/Ready/Holding overlays still describe the current item.
  - Buildings retain normal hold/resume/cancel and completed-building placement; clicking another building while construction is busy does not add a waiting request.
  - Right-clicking a waiting unit's cameo removes the newest matching waiting request without suspending or cancelling the active unit. Right-clicking the active unit's cameo retains the normal hold/cancel behavior, and the next queued item starts after an abandonment.
  - This is implemented in the standalone build only; the Remastered client already provides its own native production queue UI.
  - Queue fields, initialization, enqueue/advance logic, and classic sidebar click changes are excluded from `REMASTER_BUILD`; the DLL retains its original busy-factory behavior rather than adding a second queue behind GlyphX.

### Non-blocking explosion shake (SDL2 standalone)
- Screen shake now offsets the live rendered image over subsequent frames instead of replaying a frozen snapshot in a blocking loop. Unit simulation and input continue during building-destruction shake.
- The visual timer is renderer-local, does not consume simulation RNG, and only schedules owner/attacker shake for the appropriate local player. Remastered and other video backends retain their existing shake paths.

## Tiberian Dawn (TD)

### Commando limit
- **Komkommer:** The TD commando (**Rambo / `INFANTRY_RAMBO`**) is limited to **one per house** (enforced in `HouseClass::Can_Build`).

### Building placement proximity
- **Vanilla TD:** Base proximity/adjacency rules were stricter in ways that differed from RA.
- **Komkommer:** TD allows buildings to be placed **adjacent to friendly structures or with a one-cell gap** between the foundation and a friendly building (RA-style proximity behavior), while still requiring valid radar/placement checks.

### Nod building access
- Nod can build the **Guard Tower (`GTWR`)** and **Repair Bay (`FIX`)**.
- Base AI can fall back between faction-appropriate basic defenses and can consider a repair bay with either a War Factory or Nod Airstrip.

### Dock/service queueing (repair bays, refineries, helipads)
- **Vanilla:** Service docks using the `BuildingClass` radio/contact path could effectively block/contend in ways that made multiple units/aircraft awkward to service.
- **Komkommer:** Multiple vehicles/aircraft can **queue for service** at queueable docks that use the radio contact path:
  - **Repair bays, refineries, and helipads** are queueable in TD.
  - Harvester/unit dispatch and dock selection register and wait in queue.
  - Dock finders prefer **free or shorter queues**.

### Superweapons (skirmish/multiplayer balance)

#### Temple of Nod Nuke (`ANIM_ATOM_BLAST`)
- **Vanilla (stock):** Nuke area damage was much weaker in skirmish/multiplayer contexts (original MP damage was very low / less threatening to structures).
- **Komkommer:** Stronger nuke in **skirmish/multiplayer** (`GameToPlay != GAME_NORMAL`), tuned for base damage while keeping radius controlled:
  - Radius: **3** cells (7x7 grid)
  - Raw damage: **720**
  - Warhead: **`WARHEAD_AP`** (armor-piercing; better against structures)
  - Per-cell base damage passed to `Explosion_Damage`: `720 / ((ABS(3)/2)+1) = 360`
  - Applies scorch smudges over the affected area (existing behavior preserved with the new parameters).
- **Campaign (`GAME_NORMAL`) unchanged from Komkommer’s campaign path:** radius **4**, raw damage **1000**, warhead **`WARHEAD_FIRE`**, with the white flash/fade sequence.

#### Ion Cannon (`ANIM_ION_CANNON`)
- **Vanilla (stock):** Ion Cannon impact applied a single point blast: `Explosion_Damage(..., 600, ..., WARHEAD_PB)`.
- **Komkommer:** Ion Cannon is buffed to compete with the stronger nuke by applying **particle-beam area damage** over a focused area (all modes, as implemented):
  - Radius: **2** cells (5x5 grid)
  - Raw damage: **700**
  - Warhead: **`WARHEAD_PB`** (keeps particle-beam identity)
  - Per-cell base damage passed to `Explosion_Damage`: `700 / ((ABS(2)/2)+1) = 350`
- Ion Cannon recharge/availability rules remain the same (e.g. `ION_CANNON_GONE_TIME` 10 minutes, requires Advanced Comm Center/`STRUCT_EYE` for the normal superweapon, power suspension rules, targeting/firing flow unchanged).

## Red Alert (RA)

### Options persistence (standalone)
- Closing Game Controls with either its confirmation button or **Esc** applies and saves game speed and scroll rate; Esc no longer discards those slider changes.
- Sound Controls saves music/effects volume and score repeat/shuffle settings when the dialog closes, matching TD's existing behavior.
- RA also saves the complete options state during standalone shutdown, rather than only writing video/mouse settings there. Options load and save use the same explicit `UserPath` INI instead of resolving whichever `redalert.ini` happens to appear in the asset search path.

### Dock/service queueing (repair bays, refineries, helipads)
- **Vanilla:** Same general service contention issues for radio/contact-path docks.
- **Komkommer:** Multiple vehicles/aircraft can **queue for service** at queueable docks:
  - **Repair bays, refineries, and helipads** are queueable in RA.
  - **RA airstrip rearm is intentionally left unqueued** due to its landing/multi-slot behavior.
  - Harvester/unit dispatch and dock selection register and wait in queue; dock finders prefer free or shorter queues.

### RA repair mission targeting fix
- **Vanilla/previous behavior (bug):** RA `UnitClass` repair mission could target the wrong building (refinery instead of the repair bay) in the affected path.
- **Komkommer:** Repair mission targets the **repair bay/docking bay** correctly.

### Chronosphere targeting
- The Chronosphere can target eligible **non-friendly units**, rather than only friendly/allied units. Existing unit-type, deployment, and transport restrictions still apply.

### Soviet commando: Saboteur (skirmish/multiplayer)
- **Vanilla RA:** Tanya is the commando; commando behavior is tied to Tanya-specific handling in several places.
- **Komkommer (non-singleplayer / skirmish & multiplayer):** Commandos are split by side:
  - **Allies:** Tanya
  - **Soviets:** **Saboteur** (custom Soviet commando, `INFANTRY_SABOTEUR` type slot)
- The Saboteur is a custom Soviet commando that **reuses Counterstrike GNRL/Volkov body animations and silenced Sniper weapon behavior** where possible (silenced **Sniper** weapon/report sound, plus the SCU35EA/SCU47EA Sniper burst fix), while keeping Tanya-equivalent commando capabilities (stats/C4/capture/side/build behavior as implemented by the mod).
- **Build limit:** Commandos are limited to **one per house** in skirmish/multiplayer (`Can_Build` rejects building another if that commando type already exists for the house).
- Tanya/commando hooks were generalized (AI limits, retaliation, etc.) while keeping **single-player Tanya/hero behavior intact**.
- **Voice/feedback:** Tanya keeps her original voice/personality SFX. The Saboteur uses existing **generic infantry voice lines** (default responses) instead of Tanya’s lines; Tanya-only personality SFX (laugh/shake and Tanya-specific death vocal handling) are not applied.
- **Name/cameo:** Uses GNRL body animations with custom-cameo plumbing: tries `SBTRICON`/`SABICON`, then `GNRLICON`, then falls back to Tanya’s cameo so the sidebar never renders empty. Supports an optional `[SABOTEUR] Cameo=` (or `[GNRL]`) INI override loaded via `CCFileClass`/mix. Display name is currently `TXT_SABOTEUR` (“Saboteur”; no custom/DLL-injected string slot, so change `FullName` in `redalert/idata.cpp` to any preferred existing `TXT_*`).
- Human-controlled commandos can **auto-fire** (commando auto-fire restriction was adjusted so commandos auto-fire normally, including for human players).

### Superweapon (skirmish/multiplayer balance)

#### Atom bomb / nuke (`Do_Atom_Damage`, `ANIM_ATOM_BLAST`)
- **Vanilla (stock):** Original multiplayer nuke damage was too weak to threaten structures (much lower than campaign).
- **Komkommer:** Stronger nuke in **skirmish/multiplayer** (`Session.Type != GAME_NORMAL`), with radius rebalanced:
  - Radius: **3** (`3 * CELL_LEPTON_W` passed to `Wide_Area_Damage`)
  - Raw damage: **`(Rule.AtomDamage * 3) / 4`**
  - Warhead: **`WARHEAD_NUKE`**
- **Campaign (`GAME_NORMAL`) unchanged:** radius **4**, raw damage **`Rule.AtomDamage`**, warhead **`WARHEAD_FIRE`** (and campaign palette effects as in the remaster build guards).

## Notes
- Values above reflect the current source behavior on this branch at the time of writing.
- “Vanilla” descriptions refer to stock/original behavior as targeted by these changes; exact legacy values may differ by build/context, but the Komkommer changes listed here are the intentional deviations.
- **Save-game compatibility:** standalone production queues add `HouseClass` state; rally/service queues add `BuildingClass` state. These layout changes can invalidate saves from other builds. Queue state is now excluded from Remastered, which also changes layout relative to earlier DLLs that included it. Start fresh games when testing; do not assume cross-build save compatibility.
- Remaster mod folders: **Komkommer TD** / **Komkommer RA** (DLLs loaded by the CnC Remastered Collection). A rebuilt DLL is picked up the next time that game starts (exit the game before redeploying if the DLL is locked).
