# tests/ — three test frameworks, one tree

See `tests/README.md` for the canonical suite table; this file adds
navigation context and the custom mission-naming convention.

| Dir | Framework | Context file | What |
|---|---|---|---|
| `unit/` | Catch2 + ImGui Test Engine | `tests/unit/CLAUDE.md` | C++ unit tests, mirrors `engine/`+`apps/` source tree 1:1 |
| `integration/` | Trident (`tri`) | `tests/integration/CLAUDE.md` | SQF-driven in-game scenarios: missions, MP, UI flows, rendering |
| `e2e/` | Trident (`tri`) | `tests/e2e/CLAUDE.md` | Master-server/launcher-level scenarios |
| `perf/` | Trident (`tri`) | `tests/perf/CLAUDE.md` | Performance benchmark missions |
| `smoke/` | Pester (PowerShell) | `tests/smoke/CLAUDE.md` | Quick boot/log verification, Windows-only |
| `stress/` | Trident (`tri`) | `tests/stress/CLAUDE.md` | Long-running MP soak/chaos tests |
| `fixtures/` | — | `tests/fixtures/CLAUDE.md` | Shared test data (26 format/domain subfolders) |

## Running

```sh
# Unit tests — build first, then:
ctest --test-dir build/<preset> --output-on-failure
ctest --test-dir build/<preset> -R "<test name>" --output-on-failure

# Trident (integration/e2e/perf/stress) — build tri first:
cargo build --manifest-path engine/Trident/Cargo.toml
tri test -j6 --retries 2 tests/integration
```

Trident-backed tests need `.trident.env` (copy from `.trident.env.example`,
set `OFPR_GAME_DIR`/`OFPR_DATA_DIR` — see root `CLAUDE.md`). Unit tests
needing game data are tagged `[GameData]`/`[gamescan]`/`[external-data]` and
skip automatically when `packages/Remaster/` (or configured data dir) is absent.

## Mission file naming convention (Trident scenarios)

Extensions encode scenario type, not arbitrary choices:

| Ext | Meaning |
|---|---|
| `.Demo` | demo/campaign scenario missions (primary integration format) |
| `.test` | generic test scenario |
| `.seq` | state-sequence flow (UI state machine, e.g. menu navigation) |
| `.stress` | multiplayer stress/soak mission suite |
| `.abel` / `.cain` | combat/warfare benchmark pair (biblical Abel/Cain reference) |
| `.eden` | open-terrain/environment benchmark |
| `.noe` | urban/settlement benchmark (Noah reference) |

Each scenario typically has a companion `.toml` with tags (e.g. `["headless"]`).
