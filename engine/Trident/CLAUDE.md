# engine/Trident/ — test orchestrator (Rust)

Not a library — a standalone Rust CLI (`tri` binary, `Cargo.toml` name `tri`)
that drives integration tests. It spawns game binaries with `--harness <port>`,
connects over TCP, and feeds them SQF code/assertions to run scenario tests.
Build with `cargo build --manifest-path engine/Trident/Cargo.toml`, not CMake.

## Key files

- `src/main.rs` — CLI entry (`test`, `stress`, `console`, `exec`, `describe`, `ping` subcommands).
- `src/client/instance.rs` — `GameInstance` spawn/lifecycle management.
- `src/client/connection.rs` — `HarnessClient`, the TCP protocol implementation.
- `src/scenarios/integration.rs` — discovers and runs `*.test.sqf` files (with TOML metadata) under `tests/integration/`.
- `src/scenarios/stress.rs` — long-running multiplayer stress scenarios (`tests/stress/`).
- `protocol/` — JSON harness protocol types (newline-delimited JSON messages).
- `src/console.rs` — interactive SQF REPL against a running instance.

## Gotchas

- The harness protocol server lives **inside the game binary itself**
  (compiled into `apps/cwr`), not in this crate — `tri` is only the client/driver.
- Pure Rust, zero C++ dependency; tokio/clap/serde/tracing stack.
- Running tests needs `OFPR_GAME_DIR`/`OFPR_DATA_DIR` set (see root
  `CLAUDE.md` and `.trident.env.example`) — `tri` won't find a binary to
  spawn otherwise.
- See `tests/CLAUDE.md` for how scenarios are organized and named.
