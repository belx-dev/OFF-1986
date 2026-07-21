# mserver/CLI/ — `papa-bear-cli` (binary: `papa`)

Command-line tool for the mod/server workflow: pack/unpack PBO archives,
publish mods to the master service, query running servers, browse
server/mod catalogs.

- `src/main.rs` — entry point; subcommands: `pack`, `unpack`, `info`,
  `publish`, `list`, `install`, `query`, `servers`, `server`.
  `pack`/`unpack`/`info` call into `mserver/Archive`; `query` calls
  `mserver/Client`; `publish`/`list`/`install`/`servers`/`server` make HTTP
  calls to `mserver/MasterService`.

Deps: `papa-bear-archive`, `papa-bear-client`, `clap`, `ureq` (HTTP, native
OS cert store via `native-tls`), `zstd` (multi-threaded compression — large
mods can be 10+ GB).

## Config

- `PAPA_MASTER` env var — master-service base URL (default `https://papa-bear.cz`).
- `-k`/`--insecure` flag — bypass TLS verification (don't use against
  production endpoints; useful for local `MasterService` testing only).

Build: `cargo build` → `papa` binary. Test: `cargo test` (includes
`cli_roundtrip.rs` integration test).
