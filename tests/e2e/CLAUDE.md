# tests/e2e/ — end-to-end scenarios

Trident-driven (`tri`), same mechanism as `tests/integration/` (see
`tests/CLAUDE.md`) but for cross-cutting scenarios above single-mission
scope — e.g. `master_server_browser_visibility.test.sqf`, which exercises the
in-game server browser against the real `mserver/MasterService` master
server, not just engine-internal state.
