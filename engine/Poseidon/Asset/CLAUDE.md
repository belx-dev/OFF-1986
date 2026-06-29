# engine/Poseidon/Asset/ — binary asset loading and caching

## Subfolder map

| Dir | Contents |
|---|---|
| `Addon/` | Addon/mod registry, config parsing, dependency tracking (`AddonSystem`, `BankContext`). |
| `Cache/` | Versioned asset cache with generation tokens and memory handles (`AssetCache<T>`, `Handle`). |
| `Formats/Common/` | CSV readers, format detection (`FormatDetector`, `CsvReader`). |
| `Formats/P3D/` | ODOL v7 3D model parser (`P3DStructures`, `ODOLLoader`, `MLODLoader`/`MLODStructures`). |
| `Formats/RTM/` | Animation format reader (`BoneTransform` keyframes, `RTMReader`, `RTMVersion`). |
| `Probes/` | Asset inspector tools (`AssetInfo`, `AssetPreview`, `ShadowInspect`, `WaveToLip`, `SoundPlayer`). |

## Key files

- `Cache/AssetCache.hpp` — templated cache, `StringKey` (case-insensitive),
  O(1) `Get` via handle tokens, slot reuse.
- `Addon/AddonSystem.hpp` — static registry for addon info, preload/lock/unlock, config parsing.
- `Formats/BISStructures.hpp` — POD structures (`Vector3`/`4`, `Matrix`, `BoundingBox`, `ColorBGRA`).
- `Formats/P3D/P3DStructures.hpp` — ODOL header validation (signature `"ODOL"`, version 7), list-count bounds checks.
- `Formats/RTM/RTMReader.hpp` — animation metadata, lazy header-only reads, row-major 3x4 transforms `[aside, up, dir, pos]`.

## Gotchas

- `StringKey` is case-insensitive ASCII (matches legacy `BankArray`
  semantics) — don't switch to case-sensitive comparison without checking callers.
- `AssetCache` is explicitly **not thread-safe**; callers must serialize access.
- P3D/RTM parsers validate count bounds before allocating — this is
  deliberate hardening against malformed input (also fuzzed, see
  `apps/fuzzers` `fuzz_p3d`/`fuzz_rtm`), don't relax the checks for convenience.

## Tests

`tests/unit/engine/Poseidon/Asset/{Cache,Formats,Probes}/` — asset cache
audits, P3D/MLOD loader tests, format detection. Binary-format conversion
tests (P3D→ODOL) live separately under `tests/unit/engine/Poseidon/BinaryFormats/`.
