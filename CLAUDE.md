# Operation Frontier Fury 1985

An experimental fork of [BohemiaInteractive/CWR](https://github.com/BohemiaInteractive/CWR)
(Arma: Cold War Assault Remastered) — a large C++20 game engine (codename
**Poseidon**) plus Rust tooling (codename **Trident**) and a Rust master-server
stack (**mserver**, codename "PAPA BEAR").

This file plus a `CLAUDE.md` in most subdirectories form a navigation map for
AI agents. **Read the `CLAUDE.md` in the directory you're working in before
reading source** — it points at the load-bearing files so you don't have to
grep the whole tree. Each directory's `CLAUDE.md` complements (does not
replace) any `README.md` already there.

## Build

```sh
cmake --preset win-x64-clang-rwdi      # Windows
cmake --preset linux-x64-clang-rwdi    # Linux
cmake --build build/<preset-name>
```

- Requires `VCPKG_ROOT` set; presets auto-load vcpkg + overlay triplets/ports from `cmake/`.
- C++20 (C11 for C sources), Clang only, Ninja generator, ccache compiler launchers.
- All presets live in `cmake/presets/*.json` (included from `CMakePresets.json`). The
  full set: `win-x64-clang-{dbg,rwdi,rel,san,fuzz}`, `linux-x64-clang` (debug — note:
  no `-dbg` suffix), `linux-x64-clang-{rwdi,rel,san,tsan,fuzz}`,
  `linux-x64-steamrt4`, and `steamrt4-x64-clang-{dbg,rwdi,rel}`.
  `-san` = ASan+UBSan, `-tsan` = TSan (Linux only), `-fuzz` = libFuzzer + `POSEIDON_BUILD_FUZZERS=ON`.
- Custom build targets: `Format`/`FormatFix` (clang-format), `PythonLint`/`PythonLintFix`
  (Blender-addon Python, when tooling is available), `Lint`/`LintFix` (aggregate).
  There is **no** `Tidy` CMake target — a `.clang-tidy` config exists at the repo root
  but is not wired into the build; run clang-tidy manually if you want it.
- `POSEIDON_DISABLE_PCH=ON` disables the shared precompiled header (use to audit
  include self-containment manually — this fork has no CI, see below).
- **No CI is configured in this fork** (`.github/` contains only an issue template).
  Any "CI does X" statements you find in docs/comments are inherited from upstream
  and stale here — run formatters, tests, and sanitizer builds locally.

## Testing quick reference

```sh
ctest --test-dir build/<preset> --output-on-failure     # C++ unit tests (Catch2)
cargo test --manifest-path mserver/<Crate>/Cargo.toml   # Rust crates
cargo build --manifest-path engine/Trident/Cargo.toml   # build `tri`, then:
tri test -j6 --retries 2 tests/integration              # SQF scenario tests (needs .trident.env)
```

See `tests/CLAUDE.md` for the full test-pyramid map and data-dir requirements.

## Running / game data

Compiled binaries need game data not stored in this repo (free Demo available
on Steam: "Arma: Cold War Assault Remastered Demo"). Copy
`.trident.env.example` to `.trident.env` and set:
- `OFPR_GAME_DIR` — path to built/distributed binaries (default `dist/x64-win-rwdi`)
- `OFPR_DATA_DIR` — path to game data (default `packages/Demo`, gitignored)

Note the CMake side defaults differ: unit tests and Trident CTest registration
default the game-data dir to `packages/Remaster` (see
`tests/unit/engine/Poseidon/CMakeLists.txt`, `cmake/TridentCTest.cmake`), while
`.trident.env.example` defaults to `packages/Demo`. Point both at whatever data
you actually have.

## Top-level directory map

| Path | What | Context file |
|---|---|---|
| `apps/` | Executables: game client, demo client, dedicated server, content-pipeline tools, Blender addon, fuzzers, Tetris sample | `apps/CLAUDE.md` |
| `engine/` | Static C++ libs (Poseidon core + GL33/OpenAL backends + Formats) and the Rust `Trident` test-runner CLI | `engine/CLAUDE.md` |
| `mserver/` | Standalone Rust "PAPA BEAR" master-server stack (HTTP service, CLI, client SDK, PBO archive lib) — independent of engine/apps except wire-protocol compatibility | `mserver/CLAUDE.md` |
| `tests/` | Catch2 unit tests, Trident SQF integration scenarios, Pester smoke tests, perf/stress missions, fixtures | `tests/CLAUDE.md` |
| `thirdparty/` | Vendored `glad` (GL loader) and `renderdoc` (capture API header) — excluded from repo's GPL license | `thirdparty/CLAUDE.md` |
| `cmake/` | Presets, toolchains, vcpkg triplets/overlay ports, build helpers (file-size lint, sanitizer discovery, Catch2/Trident CTest registration) | — (see files directly, small/self-explanatory) |
| `docker/` | `papa-bear-master-service` (mserver build/runtime image) and `steamrt4` (Linux compat build environment) | — |
| `resources/` | Application icon resources only | — |
| `packages/` | Gitignored local game-data staging area (not checked in) | — |

## Modernization posture (read this before treating any rule as binding)

This fork's purpose is to **modernize the engine**. The conventions documented
in this file and in every subdirectory `CLAUDE.md` are *descriptions of the
inherited codebase* (originally written for the upstream CWR remaster), not
immutable law. Treat them as two distinct categories:

1. **Technical invariants** — things that break correctness if violated
   (e.g. static-init ordering constraints, wire-protocol compatibility with
   `mserver/`, save-format version branches, power-of-two terrain-grid math,
   determinism of `engine/Random`). Respect these unless you are deliberately
   redesigning that mechanism *and* updating every dependent listed in the docs.
2. **Legacy conventions** — inherited style/architecture choices (no-STL
   containers, `LSError` codes instead of exceptions, macro-based casting,
   module-registration patterns, etc.). These are the **default** — follow them
   for small changes so files stay internally consistent — but they are
   explicitly **open to revision**. If a refactor toward modern C++/Rust
   idioms makes the code better, propose or do it; don't contort a new design
   to fit a 2001-era pattern. When you intentionally change a convention,
   migrate coherently (whole file/subsystem, not a mixed style) and update the
   affected `CLAUDE.md` so future sessions see the new rule.

Sub-`CLAUDE.md` files mark inherited rules with wording like "*inherited
convention — see root CLAUDE.md, Modernization posture*". Where a doc says
"don't do X", read it as "X breaks something today — understand the listed
reason before doing X as part of a deliberate modernization."

## Engineering conventions that surprise newcomers

- **Legacy engine core avoids STL containers/strings** — custom `AutoArray`,
  `HashMap`, `RString` (copy-on-write), etc. live in
  `engine/Poseidon/Foundation/`. This is an *inherited convention, not a hard
  ban, and already non-uniform*: roughly a quarter of `engine/Poseidon`
  sources (newer code — asset probes, format parsers, tools-facing code)
  freely use `std::vector`/`std::string`. For routine edits, match the file
  you're editing; converting legacy code to STL is a legitimate modernization
  step when done coherently per type/subsystem (see Modernization posture
  above and `engine/Poseidon/Foundation/CLAUDE.md`).
- **Scripting language is SQF** (mission scripts), evaluated by
  `engine/Evaluator/` + `engine/Poseidon/Game/Scripting/`. Mission/test fixtures
  use custom extensions (`.Demo`, `.eden`, `.abel`, `.noe`, `.cain`, `.seq`,
  `.test`, `.stress`) — see `tests/CLAUDE.md`.
- **Three languages, three build systems in one repo:** C++ (CMake/vcpkg) for
  `engine/`+`apps/`, Rust (Cargo) for `engine/Trident` and all of `mserver/`,
  Python for the Blender addon under `apps/tools/BlenderAddon/`.
- **Test pyramid spans three frameworks:** Catch2 (C++ unit tests under
  `tests/unit/`, mirrors the source tree 1:1), Trident/`tri` (Rust CLI driving
  SQF-scripted in-game scenarios under `tests/integration/`, `tests/e2e/`,
  `tests/perf/`, `tests/stress/`), Pester (PowerShell, `tests/smoke/`, Windows-only).
- **Graphics/audio are backend-pluggable.** `engine/Poseidon/Graphics` and
  `Audio` define an abstract interface + `Dummy` no-op backend; real backends
  (`engine/PoseidonGL33`, `engine/PoseidonOpenAL`) are separate libraries
  selected at link time / via factory.
