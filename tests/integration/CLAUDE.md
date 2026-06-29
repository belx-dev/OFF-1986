# tests/integration/ — Trident SQF scenario tests

Driven by `engine/Trident`'s `tri` CLI: spawns a real game binary with
`--harness <port>`, feeds it SQF, asserts on reported state. See
`tests/CLAUDE.md` for the run command and the `.Demo`/`.test`/`.seq` naming
convention, and `engine/Trident/CLAUDE.md` / `engine/Poseidon/Dev/CLAUDE.md`
for the harness protocol itself.

| Dir | Covers |
|---|---|
| `flows/` | UI state machines — profile selection, menu navigation (`.seq`/`.test` files) |
| `ingame/` | In-mission gameplay scenarios, incl. `audio/`, `save_load/`, `vehicles/` |
| `missions/` | 15+ `.Demo` mission scenarios (e.g. `demo_end_chain.Demo`, `jip_basic.Demo`) |
| `mp/` | Multiplayer join-in-progress, horn, assignment scenarios |
| `multiplayer/` | Briefing coordination, server browser |
| `rendering/` | Rendering-output assertions |
| `scripting/` | SQF scripting-engine scenarios |
| `ui/` | Editor, main menu, options, viewer, briefing, gamepad scene catalog |

Each scenario typically has a companion `.toml` with tags (e.g.
`["headless"]`). CTest registers these per-subfolder via
`cmake/TridentCTest.cmake` (label `trident;integration`, 30-minute timeout,
run serially) — opt-in via the `OFPR_REGISTER_TRIDENT_CTESTS` CMake option.
Requires `.trident.env` set up (see root `CLAUDE.md`).
