# engine/Poseidon/IO/ — config parsing, streaming, serialization

## Subfolder map

| Dir | Contents |
|---|---|
| `Filesystem/` | `FileOps`, `DirScanner`, platform-specific implementations (`platform/`). |
| `ParamFile/` | ParamFile (game config format) parser/evaluator: CRC validation, localization, expressions, class inheritance. |
| `PreprocC/` | C-style preprocessor (`IPreproc`/`Preproc`/`PreprocC`) for macro/include support in config files. |
| `Serialization/` | Binary serialization (`SerializeBin`, `ParamArchive`), string IDs, `QStreamExt`. |
| `Streams/` | `QStream`/`QBStream` binary I/O, compression, file mapping, file info, overlapped I/O. |

## Key files

- `ParamFile/ParamFile.hpp` — main ParamFile type; `PAM` (ParamAccessMode)
  controls whether an addon/mod may override a base config entry.
- `ParamFile/ParamFileCtx.hpp` — parsing state + expression evaluation context.
- `Streams/QStream.hpp` — abstract binary stream (`QStream`/`QIOS`); errors are
  reported via the `LSError` enum (file-format version mismatch, CRC, disk
  errors), **not exceptions**.
- `Streams/FileCompress.hpp` — compression wrapping (`FileCompress`/`SsCompress`), transparent over `QStream`.
- `PreprocC/PreprocC.hpp` — macro/include preprocessing, can be chained in front of ParamFile parsing.
- `Serialization/ParamArchive.hpp` — binary archive mirroring ParamFile structure, used for save/load.

## Gotchas

- ParamFile classes inherit from a parent class; `PAM` is what gates whether
  a mod's config is even allowed to touch a given base entry — check this
  before assuming a config override will "just work."
- `QStream` subclasses implement platform-specific file ops and use `LSError`
  return codes, not exceptions *(inherited convention — see root `CLAUDE.md`,
  Modernization posture; today's callers only check `LSError`, so mixing in
  exceptions piecemeal breaks error propagation — an error-handling
  modernization would have to convert the `QStream` call graph coherently)*.
- `SerializeBin` formats are endian-aware and version/checksum-tagged; bump
  version deliberately if you change a serialized struct's layout.

## Tests

`tests/unit/engine/Poseidon/IO/{Filesystem,ParamFile,PreprocC,Streams}/` —
ParamFile parsing/inheritance/expressions, QStream compression/PBO
roundtrips/banks, preprocessing, FileOps/DirScanner. `server_admin_login_poc`
shows realistic ParamFile usage end-to-end.
