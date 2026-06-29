# tests/stress/ — multiplayer soak/chaos tests

Trident-driven (`tri`), long-running stability tests under `mp/`. Each
`*.stress` directory has a `hooks/` subfolder defining injected failure
behaviors (crash faults, network latency, node restarts):

- `basic_soak.stress` — baseline long-duration stability.
- `von_hosted_two_player.stress` / `von_soak.stress` — voice-chat (VoN) load.
- `fault_latency.stress` — injected network latency.
- `jip_churn.stress` — repeated join-in-progress churn.
- `restart_recovery.stress` — server restart/recovery behavior.

These run for extended durations (hours, not minutes) — don't expect them in
a normal CI feedback loop; run targeted ones manually when touching
networking, VoN, or server lifecycle code.
