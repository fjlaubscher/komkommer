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

### Credits counter on the sidebar tab (standalone)
- **Vanilla:** The top bar had a dedicated credits tab (with the scrolling credit counter) beside a separate sidebar-toggle tab labelled "Sidebar".
- **Komkommer:** The credit count renders on the **sidebar-toggle tab** at the top-right instead of the word "Sidebar". Clicking that tab still collapses/expands the sidebar, and the counter keeps its classic tick-up/tick-down animation and money sounds. The old separate credits tab is gone, which also moves the counter out from under the macOS screen notch.
- RA's "insufficient funds" flash now flashes that right-edge tab. RA with `AllowSidebarToggle=false` (sidebar always visible) already showed its credits tab on the right edge and is unchanged; the RA mission-timer tab keeps its position. Remastered host UI is unchanged.

### Campaign autosave (standalone)
- Each campaign mission writes `SAVEGAME.999` when it starts. The next mission overwrites that file. Skirmish, multiplayer, the scenario editor, and loading an existing save do not.
- The load list shows `AUTOSAVE: NOD 01` (GDI, SOVIET, or ALLIES the same way). Remastered keeps the host's own save UI.

### Aircraft scouting and loiter
- Aircraft can be ordered to move to or attack non-visible (fogged/shrouded) cells for scouting; reveal comes from their normal sight while airborne.
- Aircraft sight defaults to **3 cells** in both engines. Stock RA `Sight=0` entries are raised to that baseline after rules loading, including helicopters; non-zero INI sight values still override it. The TD airstrip delivery plane (`C17`) stays at **0** so its flight does not reveal the map.
- Terrain-reveal call sites use the type's sight range: TD ground-layer entry/firing, and RA `Look()`/firing. Grounded RA aircraft are no longer clamped to a one-cell sight range.
- **Helicopters:** a move order to terrain flies to the destination and holds/loiters there to scout instead of auto-landing on arrival. The self-action (deploy) orders an airborne helicopter to land (seeking a valid helipad/LZ context); a landed transport with passengers unloads. A landed empty helicopter has no self-action — order it to move to take off. A new move order while grounded or landing lifts off and flies to the new destination. **Transport helicopters (the Chinook) do not loiter:** their move orders — including into fog — fly to the destination and land there, matching vanilla transport behavior; their sight while airborne is unchanged.
- **RA Yak/MiG:** a move order to a scout point flies to and circles/loiters around that area to reveal it, without landing on terrain, until given a new order or directed to return/dock/reload. Existing airstrip docking/reload/ammo/return behavior is preserved. Scripted/non-scout fixed-wing behavior (e.g. TD A-10, RA cargo planes) is unchanged.

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

### Sound playback (standalone)
- **Vanilla:** At most 5 sound effects could play at once; starting a sound above that limit immediately cut off the lowest-priority one, and the OpenAL mixer could also leave a stale buffer queued after such a cut-off, glitching the replacement sound.
- **Komkommer:** The standalone OpenAL mixer now allows up to **16** simultaneous sound effects, and stopping a sample fully unqueues its OpenAL buffers so a voice-stolen sound starts cleanly. Remastered (DirectSound) playback is unchanged.

### Pathfinding (A*)
- **Vanilla:** `Find_Path` walked a straight line and, on hitting an impassable cell, "edge-followed" the obstacle clockwise/counterclockwise. It could route around obstacles on the long side, wander, and fail in cul-de-sacs, doughnut-shaped obstacles, and maze-like base layouts.
- **Komkommer:** `Find_Path` uses an **A\*** search over the cell grid (8-direction movement, Chebyshev heuristic, binary-heap open list) that finds the optimal route under the game's movement-cost model.
  - Cell passability and cost still come from the existing `Passable_Cell`, so the caller's movement threshold (`MOVE_CLOAK` → `MOVE_TEMP`) and roundabout-team threat avoidance are preserved.
  - Diagonal moves never cut a corner through an occupied cell. Impassable ground beside the step (water, rock, wall, and RA river) does not count, so a one-cell diagonal bridge, cliff gap, or wall gap stays connected. A destroyed span stays blocked because the gap cell itself is impassable.
  - If the destination is unreachable, the search still returns a best-effort path to the closest reachable cell, so units make progress and re-path instead of stalling.
  - The command list is still smoothed by `Optimize_Moves`. A 90° or 135° shortcut is kept only when that cell is passable.
- This changes only the path search itself; the `PathType` command-list contract and the movement/`drive.cpp` consumers are unchanged.

### Harvester destination choice
- **Vanilla:** Autonomous harvesters stop at the first ore ring that contains any cell, return after a dump to the cell they just emptied, and stick to the last refinery they used even when another dock is closer or freer.
- **Komkommer:** When a harvester chooses for itself (a player click on a field still wins while that destination is set):
  - It still walks rings outward and stops at the **nearest ring that has ore**, then picks the **richest** cell on that ring. It skips a cell another friendly harvester is already driving to.
  - When the hold fills, it stores a nearby cell that **still has ore** as the return point (TD short scan is 6 cells; RA uses `OreNearScan` / `TiberiumShortScan`).
  - It picks a refinery by **distance plus a fixed wait per other harvester already bound there** (13 cells per load in TD, 6 in RA). A truck already within 5 cells of its current refinery discounts that dock's wait so it does not U-turn over a one-load swing. The dock service queue itself remains FIFO.

## Tiberian Dawn (TD)

### Skirmish setup (standalone)
- The standalone skirmish screen lists maps by playable area, smallest first, then alphabetically. Each label includes capacity (`2P`, `4P`, etc.); the selected map also shows its dimensions and maximum players. Capacity comes from distinct starting waypoints inside the playable map, capped at six players, rather than guessing from the map's name.
- The roster supports the human plus up to five AI players, limited by the selected map. Faction, colour, and team fields all use dropdowns. Human factions are GDI/Nod; AI factions also offer Random. Teams offer None and 1–6, and colours offer all six colours. Deliberately choosing an occupied colour swaps only those two players' colours, without cycling through or disturbing intermediate colours. Choosing an unused colour leaves other active players unchanged. Only one roster dropdown is open at a time; **Esc** dismisses it without leaving setup. Use **Add AI** or a row's **-** button to change opponents. Selecting a smaller map removes excess AI from the active roster.
- **None** means free-for-all: two players with no team are enemies. Matching numbered teams create mutual alliances. A match needs at least one opponent of the human before **Start** is enabled. The legacy AI-paranoia routine no longer overrides standalone skirmish alliances after an AI defeat; the existing allied-survivor victory check is retained.
- Difficulty remains one **Match difficulty** setting using TD's existing human/computer handicaps and bonuses, not separate AI intelligence levels. Credits, starting units, tech level, bases, tiberium regrowth, crates, and capture-the-flag controls remain available.
- The footer places **Back** on the left, **Restore defaults** in the middle, and **Start** on the right. Leaving with **Start**, **Back**, or **Esc** saves the map identity, complete roster, player name, difficulty, and rules in `[Skirmish]` in `UserPath/conquer.ini`. **Restore defaults** resets the match setup and selects the smallest map while retaining the entered name. Other config sections are preserved.
- Startup filters duplicate/out-of-bounds skirmish starting waypoints and initializes unused start slots. The waypoint sorter no longer reads one entry beyond its available inputs. New standalone skirmish recordings include a tagged roster/difficulty extension; loading an older recording does not read settings from the current lobby config. Recording playback and gameplay still require manual testing.
- Network lobbies and the Remastered host UI are unchanged. RA has a corresponding standalone lobby described below. No saved engine-class fields were added. Compilation and settings/metadata unit tests do not substitute for testing roster controls, actual team combat/victory, persistence after relaunch, and classic rendering in game.

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
  - An empty harvester is turned away by the refinery radio check, the same way a repair bay turns away a unit that does not need service. Waiting in the queue does not replace the harvester's last ore cell, so after a real dump it returns to the field.
  - When a repair bay kicks a unit off (`RADIO_RUN_AWAY`), the unit picks a scored exit cell around the plus-shaped pad (prefer clear, then temporarily blocked) and asks allied blockers to scatter if needed, instead of only scattering into whatever is adjacent.
  - A human unit that cannot start repair (typically no cash) is kicked off the pad the same way AI units already were, so a broke queue does not sit on the bay forever.

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

### Skirmish setup (standalone)
- Uses the same design as TD: maps sorted by playable area (smallest first), capacity in each map label, selected-map dimensions, an editable roster, and one global match difficulty using RA's existing handicaps/bonuses. RA's footer places **Back** on the left, **Restore defaults** in the middle, and **Start** on the right.
- Supports the human plus up to seven AI, limited by the map's distinct playable start waypoints (indices 0–7) and the rules' player limit. Country dropdowns retain the stock human choices: **USSR, England, Ukraine, Germany, France**. AI also offers **Random**, retaining the engine's existing seeded Allied/Soviet random choice. Colour dropdowns offer all eight RA colours; choosing an occupied colour swaps only those two slots. Team dropdowns offer **None / 1–8**, with None meaning free-for-all and matching numbered teams allied.
- RA retains its own native palette/widgets and an eight-row layout. The map list extends down the left; credits/units, tech/difficulty, and two columns of rule checkboxes form a compact block below the roster on the right. Content and footer buttons are inset from RA's metal frame rather than drawn over its side rails or bottom border. Only one roster dropdown opens at a time; **Esc** dismisses it before leaving the screen. Selecting a smaller map removes excess AI from the active roster. Start requires at least one enemy of the human. Geometry tests check the shared layout coordinates at 320×200 and 640×400, including all eight rows and popup bounds; actual text/rendering still needs an in-game check.
- Starting credits (including rules-defined defaults/maximum), units, tech level, bases, **ore regrowth**, crates, **shadow regrowth**, and capture-the-flag settings persist with the map filename, name, full roster, and difficulty in `[Skirmish]` in the explicit `UserPath/redalert.ini`. Start, Back, and Esc save preferences; Restore defaults retains the entered name and selects the smallest map. Other INI sections are preserved.
- Map discovery still uses RA's session list and engine asset lookup, including MIX archives and custom MPR maps. Expansion-hosting restrictions are retained. Sorting/filtering the display does not change which original session map is launched, and persisted selections use filenames rather than list positions.
- Startup applies the selected countries, colours, and mutual teams without adding saved-class fields. Player teams override map-defined multiplayer alliances, while non-player/neutral alliances remain intact. AI paranoia no longer replaces standalone skirmish teams after a defeat. Official maps retain their first-four/corner starting-position preference where enough valid starts exist; sparse valid starts are accepted, and small maps no longer need invented extra positions when their actual roster already fits.
- New standalone skirmish recordings carry a tagged roster/difficulty extension; older recording headers remain readable without consulting current lobby preferences. Recording playback, country behaviour, team combat/victory, eight-player rendering, and relaunch persistence still require in-game testing. Both standalone games and both x86 Remastered DLLs compile, and the CTest suite covers TD regression cases plus RA preferences, eight-slot colour/team handling, map metadata, and sorting—not full gameplay.
- Shared helpers in `common/skirmishroster.h` and `common/skirmishdrop.h` provide roster/colour/team logic and native dropdown behaviour for both standalone games. Map formats, country enums, palettes, rules, session setup, and recording integration stay in each engine. Campaign, network lobby UI, and Remastered host UI are not replaced.

### Options persistence (standalone)
- Closing Game Controls with either its confirmation button or **Esc** applies and saves game speed and scroll rate; Esc no longer discards those slider changes.
- Sound Controls saves music/effects volume and score repeat/shuffle settings when the dialog closes, matching TD's existing behavior.
- RA also saves the complete options state during standalone shutdown, rather than only writing video/mouse settings there. Options load and save use the same explicit `UserPath` INI instead of resolving whichever `redalert.ini` happens to appear in the asset search path.
- Music volume is a single persistent value across campaign and skirmish/multiplayer. Vanilla RA kept a separate `MultiplayerScoreVolume` (saving `ScoreVolume` only in campaign and overwriting it with the multiplayer value on skirmish/MP start), which made music volume appear to reset after relaunch.

### Dock/service queueing (repair bays, refineries, helipads)
- **Vanilla:** Same general service contention issues for radio/contact-path docks.
- **Komkommer:** Multiple vehicles/aircraft can **queue for service** at queueable docks:
  - **Repair bays, refineries, and helipads** are queueable in RA.
  - **RA airstrip rearm is intentionally left unqueued** due to its landing/multi-slot behavior.
  - Harvester/unit dispatch and dock selection register and wait in queue; dock finders prefer free or shorter queues.
  - An empty harvester is turned away by the refinery radio check, the same way a repair bay turns away a unit that does not need service. Waiting in the queue does not replace the harvester's last ore cell, so after a real dump it returns to the field.
  - When a repair bay kicks a unit off (`RADIO_RUN_AWAY`), the unit picks a scored exit cell around the plus-shaped pad (prefer clear, then temporarily blocked) and asks allied blockers to scatter if needed, instead of only scattering into whatever is adjacent.
  - A unit that cannot start repair (typically no cash) is kicked off the pad in IDLE as well as mid-repair, so the bay does not stay blocked.

### RA repair mission targeting fix
- **Vanilla/previous behavior (bug):** RA `UnitClass` repair mission could target the wrong building (refinery instead of the repair bay) in the affected path.
- **Komkommer:** Repair mission targets the **repair bay/docking bay** correctly.

### Chronosphere targeting
- The Chronosphere can target eligible **non-friendly units**, rather than only friendly/allied units. Existing unit-type, deployment, and transport restrictions still apply.

### Engineer capture
- **Vanilla RA:** An engineer chips an enemy building (about a third of its strength per entry) until health is at or below `EngineerCaptureLevel` (red), then captures it. The cursor is the damage cursor until then.
- **Komkommer:** Entering a capturable enemy building captures it at any health, matching Tiberian Dawn, in campaign and skirmish/multiplayer. The cursor is the capture cursor immediately. Friendly repair is unchanged. A building that cannot be captured still takes the chip damage. `EngineerCaptureLevel` no longer affects the outcome.

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
