# engine/PoseidonFormats/ — asset-format C API

Unified C API for reading P3D models, PAA/PAC textures, PBO archives, and RTM
animations, built both as a static lib and as `PoseidonFormatsDLL` (x64-only
shared lib) for cross-language consumers.

- `PoseidonFormats.h` — public C API, 40+ functions covering model / image /
  archive / VFS / animation access.
- `PoseidonFormats.cpp` — implementation; wraps `Poseidon`'s own P3D loaders,
  PAA decoders, and RTM readers rather than reimplementing parsing.

Depends on `Poseidon` (engine core, for the asset loaders + graphics-dummy
backend it needs to construct them) and `winmm` on the DLL build.

Consumed by: `apps/tools/BlenderAddon` (loads the `.dll`), the Total Commander
plugins in `apps/tools/TcPbo`/`TcLister`, and `apps/tools/Tools`/`Studio`.
If you change the binary layout of P3D/PAA/PBO/RTM parsing in
`engine/Poseidon/Asset/Formats/`, check whether this API's surface needs
updating too — it's a separate, stability-sensitive boundary for external tools.
