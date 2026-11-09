# Game and mod resources

`remaster_mods/komkommer_TD` and `remaster_mods/komkommer_RA` are **source templates**, not installed mod copies. CMake copies them into the build tree and stages DLLs under `Data/`. Edit source assets/manifests here, not generated/deployed copies.

## Asset boundaries

- Keep each mod's `ccmod.json` at its root, its DLL/assets in `Data/`, and TD's `GameConstants_Mod.xml` alongside the manifest. Deploy the complete folder rather than only a DLL when assets change.
- Classic and Remastered HD resources are different pipelines. RA's `SBTRICON.SHP` is a classic sidebar icon; naming another file `SBTRICON-hd.SHP` does not register a new HD icon with GlyphX. The ineffective file was removed in `5bc064c`.
- RA's `idata.cpp` loads cameo pointers, while `dllinterface.cpp` exports sidebar asset names. Check both before concluding an icon is fixed. The HD Saboteur export currently uses Tanya's `E7ICON` fallback; do not promise a custom HD cameo from the classic asset alone.
- SHP conversion needs the correct indexed game palette (the Saboteur work used `TEMPERAT.PAL`). See `tools/png_to_shptd.py` and inspect its actual input requirements before using it; do not assume a generic RGBA image is ready for in-game use.
- Test readable cameo/name, correct palette/transparency, classic rendering, and HD rendering independently when relevant.

Do not commit extracted game data, developer screenshots/raw intermediates, built DLLs/PDBs, or credentials. Preserve existing manifests and asset names unless intentionally changing the resource contract.
