# Komkommer test maps (RA + TD, 1P sandbox)

Ready-made **local copies** were created in your Remastered custom maps folders (copied from existing local maps; originals left unchanged). Launch with the matching mod enabled: **Komkommer RA** or **Komkommer TD**.

## Red Alert — Komkommer RA 1P Test (Russia vs Germany)
- **Files:** `C:\Users\franc\OneDrive\Documents\CnCRemastered\Local_Custom_Maps\Red_Alert\KOMKOMMER_RA_1P_TEST.MPR` (+ `.JSON`)
- **Setup:** `Player=USSR`, `USSR Credits=50000`, `Germany Credits=10000`, `BuildLevel=99`, terrain/objects copied from the base map.
- **Tip:** For a clean sandbox, open this map in the **RA Map Editor** and place a compact USSR base (ConYard/Power/Refinery+Harvester/Barracks/War Factory/Radar/Repair Depot/Helipad) and a small German base/opponent if desired.

### Komkommer RA features to test
- **Rally points:** select a production/service building, **left-click** a destination. New/repaired/serviced units move to the rally cell; the animated flag shows only while selected.
- **Saboteur (skirmish/MP, Soviets):** Allies=Tanya, Soviets=**Saboteur** (1/house). GNRL body anims, silenced Sniper behavior, C4/capture, human auto-fire. Build as Soviets, or pre-place via `[INFANTRY]` as `USSR,GNRL,256,<cell>,0,GUARD,64,None`.
- **Service queues:** repair depots/refineries/helipads queue (RA airstrip rearm intentionally unqueued). Repair targeting should go to the repair bay.
- **RA nuke (skirmish/MP):** radius 3, `(AtomDamage*3)/4`, `NUKE` warhead.

## Tiberian Dawn — Komkommer TD 1P Test (Nod vs GDI)
- **Files:** `C:\Users\franc\OneDrive\Documents\CnCRemastered\Local_Custom_Maps\Tiberian_Dawn\KOMKOMMER_TD_1P_TEST_MAPDATA.INI/BIN/JSON/TGA`
- **Setup:** `Player=BadGuy` (Nod), `BadGuy Credits=50000`, `GoodGuy Credits=10000`, `BuildLevel=99`.
- **Tip:** Open in the **TD Map Editor** and place a Nod starting base (Construction Yard/Hand of Nod/Power/Airstrip/Refinery) so you can immediately test the new Nod build options. Add a small GDI presence if you want targets.

### Komkommer TD features to test (including recent changes)
- **Nod can build Guard Tower (GTWR):** Nod should have access to the GDI-style Guard Tower (Hand of Nod/Barracks prereq is satisfied by Nod’s Hand; 1x1 defense). Test build/sidebar availability and in-game behavior.
- **Nod can build/use Repair Bay (FIX):** Repair Bay is available to Nod (Power prereq). Place/repair vehicles; confirm docking/repair works. Komkommer base AI can also consider building a repair bay when a vehicle facility exists (`WEAP` or Nod `AIRSTRIP`).
- **Rally points:** select a production building, **left-click** a destination; flag shows only while selected.
- **TD superweapons:** Ion Cannon area PB damage (5x5, tuned) and Nod nuke balance in skirmish/MP as per Komkommer changes.
- **Other Komkommer TD behavior:** commando (Rambo) 1/house, RA-style building proximity (adjacent or 1-cell gap), dock/service queueing for repair/refinery/helipad.

## Notes
- These are **sandbox copies** for testing mod features; terrain/bases may be minimal depending on the source map. Use the in-game Map Editor to add starting structures/units as needed.
- Some Komkommer behaviors are **skirmish/multiplayer-specific** (RA Saboteur split/limit, superweapon balance in skirmish/MP). If a feature doesn’t appear in a 1P/custom flow, test it in Skirmish with the mod enabled.
- Back up/copy maps before further edits. After rebuilding/deploying a DLL, restart the game to load the new mod DLL.