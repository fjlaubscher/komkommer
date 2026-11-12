# Game and mod resources

`remaster_mods/komkommer_TD` and `remaster_mods/komkommer_RA` are source templates. CMake copies them into the build. Edit them here, not a generated or installed copy.

- Each mod has `ccmod.json` at its root and assets in `Data/`. TD also has `GameConstants_Mod.xml` beside the manifest. Deploy the folder, not a lone DLL, when assets change.
- A classic SHP and a Remastered HD asset are different pipelines. Naming a file `SBTRICON-hd.SHP` does not register an HD icon. `idata.cpp` loads the classic cameo pointer; `dllinterface.cpp` exports the HD asset name. Saboteur's HD name falls back to Tanya's `E7ICON`.
- SHP conversion needs the indexed game palette (`TEMPERAT.PAL` for that cameo), not an RGBA PNG. See `tools/png_to_shptd.py`.
- Do not commit extracted game data, screenshots, intermediates, DLLs, PDBs, or credentials.
