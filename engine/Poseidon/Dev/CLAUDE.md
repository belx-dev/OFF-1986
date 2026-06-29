# engine/Poseidon/Dev/ — debug tools, diagnostics, test harness

## Subfolder map

| Dir | Contents |
|---|---|
| `Debug/` | `DebugCheats`, `DebugCommands` (console command registry), `DebugOverlay` (in-game HUD), `DebugTrap`, `DebugWin` (inspector), `Profiler`. |
| `Diag/` | `FrameProfiler`, `ScopedTimer` (RAII timer), `PerfTrace` (VTune/ITT integration), `VtuneProf`, `DiagModes`. |
| `Harness/` | `HarnessServer` and supporting trackers — the in-game side of the automated test protocol that `engine/Trident` drives. |

## Key files

- `Debug/DebugCommands.hpp` — console command registration/invocation.
- `Debug/DebugOverlay.hpp` — in-game debug HUD.
- `Diag/FrameProfiler.hpp` — per-frame CPU/GPU timing.
- `Diag/ScopedTimer.hpp` — RAII scope timer for profiling hot regions.
- `Diag/PerfTrace.hpp` — performance tracing with VTune ITT instrumentation.
- `Harness/HarnessServer.hpp` — listens for `--harness <port>` connections from
  `engine/Trident`'s `tri` CLI; exposes mission-state/player-state queries
  (`HarnessMissionStateTracker`, `HarnessPlayerTracker`) and a text-ish protocol
  for driving SQF and reading state.

## Gotchas

- The harness protocol here is the **server side**; the Rust client side
  lives in `engine/Trident/src/client/connection.rs`. If you change one, check
  the other — they're not type-shared, just protocol-compatible.
- Debug overlay/commands are typically compiled out or gated in release builds.

## Tests

`tests/unit/engine/Poseidon/Dev/{Debug,Diag}/` — debug commands, debug
profiling, dev-mode debug-vs-release gating.
