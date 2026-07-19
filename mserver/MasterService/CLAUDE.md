# mserver/MasterService/ — `papa-bear-master-service`

The PAPA BEAR master server: HTTP service for game-server
registration/heartbeat, reachability probing, mod catalog/distribution, and a
server-rendered web browser UI. Dual-mode binary: `server` (HTTP router +
SQLite/Postgres) and `probe` (standalone reachability-monitoring daemon,
typically deployed separately).

## Key files

- `src/main.rs` — CLI entry; `server`/`probe` subcommands.
- `src/http.rs` — Axum router, all `/v1/*` API endpoints + web-shell routes.
- `src/lib.rs` — exported modules: `service`, `repository`, `model`, `mods`, `dev_seed`.
- `src/service.rs` — business logic (register/heartbeat/list/observe servers; list/get/publish mods).
- `src/repository.rs` — `SqliteServerDirectory` (SQLite + schema migration;
  also supports Postgres via SQLx's `Any` pool with placeholder rewriting).
- `src/mods.rs` — `ModStore` (local filesystem or S3 via `object_store`).
- `src/probe.rs` — reachability prober; uses `mserver/Client`'s `query_server()`.
- `web/` — HTML shell pages (`index.html`, `browser.html`, `browser-detail.html`,
  `mods.html`, `mod-detail.html`) + `web/assets/` (`papa-bear.js` client-side
  nav/polling, `site.css`).
- `dev-data/dev_mods_seed.csv` — 120-row (plus header) synthetic seed dataset
  for local `--dev` startup.

## Config (CLI flags)

`--db`, `--mods-dir`, `--admin-api-key`, `--client-ip-header` (e.g.
`X-Forwarded-For`), `--require-server-token`, `--dev` (seed DB on startup).
Download URLs are made absolute using `Host` + `X-Forwarded-Proto` headers —
expects to run behind a reverse proxy in production.

## Build/test/run

`cargo test` runs 40+ integration tests (HTTP routes, DB migrations, mod
upload/delete, server-observation state machine, OpenAPI contract via
`utoipa`). Docker: `docker/papa-bear-master-service/Dockerfile` (multi-stage
Rust 1-bookworm builder → distroless `cc-debian12` nonroot runtime; serves on
port 8080; SQLite at `/data/papa-bear.sqlite3` by default).

Deps: `axum`, `tokio`, `sqlx` (rustls TLS; SQLite/Postgres/`Any` dialects),
`object_store`, `serde_json`, `csv`, `utoipa`, `papa-bear-client`.
