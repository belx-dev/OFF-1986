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
- Other presets: `*-dbg`, `*-rel`, `*-san` (ASan+UBSan), `*-tsan` (Linux TSan), `*-fuzz` (libFuzzer), `linux-x64-steamrt4`.
- Custom build targets: `Format`/`FormatFix` (clang-format), `Tidy`/`TidyFix` (clang-tidy), `Lint`/`LintFix`.
- `POSEIDON_DISABLE_PCH=ON` disables the shared precompiled header (use to audit include self-containment; CI runs this way).

## Running / game data

Compiled binaries need game data not stored in this repo (free Demo available
on Steam: "Arma: Cold War Assault Remastered Demo"). Copy
`.trident.env.example` to `.trident.env` and set:
- `OFPR_GAME_DIR` — path to built/distributed binaries (default `dist/x64-win-rwdi`)
- `OFPR_DATA_DIR` — path to game data (default `packages/Demo`, gitignored)

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

## Engineering conventions that surprise newcomers

- **No STL containers/strings in engine/Poseidon.** Custom `AutoArray`,
  `HashMap`, `RString` (copy-on-write), etc. live in
  `engine/Poseidon/Foundation/`. Don't introduce `std::vector`/`std::string`
  into Poseidon code — match existing container usage in the file you're editing.
  See `engine/Poseidon/Foundation/CLAUDE.md`.
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
