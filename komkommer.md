# Komkommer mod behavior changes (vs stock vanilla)

This document lists gameplay/behavior changes introduced by the **Komkommer** mod compared to stock vanilla **Tiberian Dawn (TD)** and **Red Alert (RA)** behavior in this codebase. It focuses on player-visible/gameplay differences (not rebranding, CI, build/tooling, or internal refactors except where they affect behavior).

## Both games (TD + RA)

### Rally points for production buildings
- **Vanilla:** Production buildings do not have a player-settable rally point workflow like this mod’s implementation.
- **Komkommer:** Production buildings support rally points.
  - A left click stores a rally cell through the event queue.
  - Newly produced and fully repaired units move to the rally point.
  - The rally marker is shown as a **player-coloured animated flag** (uses the cell flag/animation system so it renders correctly in both classic and remastered graphics modes).
  - The rally flag is shown **only while the structure is selected**.
  - Rally flags do **not** block structure placement (flagged rally cells are ignored for placement blocking where appropriate).

## Tiberian Dawn (TD)

### Commando limit
- **Komkommer:** The TD commando (**Rambo / `INFANTRY_RAMBO`**) is limited to **one per house** (enforced in `HouseClass::Can_Build`).

### Building placement proximity
- **Vanilla TD:** Base proximity/adjacency rules were stricter in ways that differed from RA.
- **Komkommer:** TD allows buildings to be placed **adjacent to friendly structures or with a one-cell gap** between the foundation and a friendly building (RA-style proximity behavior), while still requiring valid radar/placement checks.

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

### Dock/service queueing (repair bays, refineries, helipads)
- **Vanilla:** Same general service contention issues for radio/contact-path docks.
- **Komkommer:** Multiple vehicles/aircraft can **queue for service** at queueable docks:
  - **Repair bays, refineries, and helipads** are queueable in RA.
  - **RA airstrip rearm is intentionally left unqueued** due to its landing/multi-slot behavior.
  - Harvester/unit dispatch and dock selection register and wait in queue; dock finders prefer free or shorter queues.

### RA repair mission targeting fix
- **Vanilla/previous behavior (bug):** RA `UnitClass` repair mission could target the wrong building (refinery instead of the repair bay) in the affected path.
- **Komkommer:** Repair mission targets the **repair bay/docking bay** correctly.

### Soviet commando: Volkov (skirmish/multiplayer)
- **Vanilla RA:** Tanya is the commando; commando behavior is tied to Tanya-specific handling in several places.
- **Komkommer (non-singleplayer / skirmish & multiplayer):** Commandos are split by side:
  - **Allies:** Tanya
  - **Soviets:** **Volkov** (Soviet commando)
- Volkov is set up as a **Tanya-equivalent** in non-singleplayer modes (matched stats, weapons, C4/capture/commando behavior as implemented by the mod).
- **Build limit:** Commandos are limited to **one per house** in skirmish/multiplayer (`Can_Build` rejects building another if that commando type already exists for the house).
- Tanya/Volkov/commando hooks were generalized (AI limits, retaliation, etc.) while keeping **single-player Tanya/hero behavior intact**.
- **Voice/feedback:** Tanya keeps her original voice/personality SFX. Volkov uses existing **generic infantry voice lines** (default responses) instead of Tanya’s lines; Tanya-only personality SFX (laugh/shake and Tanya-specific death vocal handling) are not applied to Volkov.
- **Name/cameo:** Volkov uses a **“Saboteur”** display label in skirmish/multiplayer (placeholder where a dedicated Volkov string/asset is not available). If Volkov’s own sidebar cameo data is unavailable, the mod falls back to Tanya’s cameo data so a sidebar icon is shown.
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
- Remaster mod folders: **Komkommer TD** / **Komkommer RA** (DLLs loaded by the CnC Remastered Collection). A rebuilt DLL is picked up the next time that game starts (exit the game before redeploying if the DLL is locked).
