# tests/stress/ — multiplayer soak/chaos tests

Trident-driven (`tri`), long-running stability tests under `mp/`. Each
`*.stress` directory has a `stress.toml` defining instances/phases; failure
injection is expressed via `[phases.before_actions]`
(`start_role`/`stop_role`/`restart_role` — churn/restart) or
`[phases.toxics]` (toxiproxy-backed network faults, e.g. latency). Some
scenarios also have a `hooks/` subfolder of SQF assertion scripts run via
`[phases.after_actions]` to verify post-phase state:

- `basic_soak.stress` — baseline long-duration stability.
- `von_hosted_two_player.stress` / `von_soak.stress` — voice-chat (VoN) load.
- `fault_latency.stress` — injected network latency.
- `jip_churn.stress` — repeated join-in-progress churn.
- `restart_recovery.stress` — server restart/recovery behavior.

These run for extended durations (hours, not minutes) — don't expect them in
a normal CI feedback loop; run targeted ones manually when touching
networking, VoN, or server lifecycle code.
