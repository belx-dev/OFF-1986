# mserver/ — "PAPA BEAR" master-server stack (Rust)

Four independent Rust crates (Cargo.toml each, MIT-licensed, Rust 1.79+).
**Completely standalone from `engine/`/`apps/`** — no build/code dependency,
only wire-protocol compatibility (the C++ engine implements the protocol
these crates also implement/consume independently; see
`engine/Poseidon/Network/CLAUDE.md`). This fork has no CI — run
`cargo fmt --check`, `cargo clippy`, and `cargo test` locally before pushing;
all four crates deny `unsafe_code` and `clippy::all`, and warn on
`pedantic`/`nursery` via `[lints]` in each `Cargo.toml`.

| Crate | Dir | Context file | Role |
|---|---|---|---|
| `papa-bear-archive` | `Archive/` | `mserver/Archive/CLAUDE.md` | PBO archive codec (standalone, zero cross-crate deps) |
| `papa-bear-client` | `Client/` | `mserver/Client/CLAUDE.md` | UDP query client SDK for probing game-server status (standalone) |
| `papa-bear-cli` (`papa`) | `CLI/` | `mserver/CLI/CLAUDE.md` | CLI: pack/unpack PBOs, publish/install mods, query servers — depends on Archive + Client |
| `papa-bear-master-service` | `MasterService/` | `mserver/MasterService/CLAUDE.md` | HTTP service: server registry/heartbeat, mod catalog, web UI — depends on Client (for its `probe` mode) |

## Dependency graph

```
Archive  (standalone)
Client   (standalone)
CLI      → Archive, Client, + HTTP calls to MasterService
MasterService → Client (probe mode only)
```

Build/test any crate with `cargo build`/`cargo test` from its own directory,
or via `--manifest-path mserver/<Crate>/Cargo.toml` from the repo root.
Docker image for MasterService: `docker/papa-bear-master-service/Dockerfile`
(build context limited to `Client/` + `MasterService/` via `mserver/.dockerignore`).
