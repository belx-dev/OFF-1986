# tests/perf/ — performance benchmark missions

Trident-driven (`tri`) benchmark missions under `missions/`, each a mission
directory (`mission.sqm` + assets) with a name encoding scenario type (see
`tests/CLAUDE.md` for the extension convention):

- `perf_abel.abel` — combat-focused benchmark.
- `perf_combat.eden` — combat in open terrain.
- `perf_field.eden` — open-field terrain benchmark.
- `perf_town.noe` — urban/settlement density benchmark.
- `perf_water.eden` — water/coastal rendering benchmark.

Used for tracking frame-time/CPU regressions across changes to `World/`,
`Graphics/`, or `AI/` — if you're optimizing simulation or rendering hot
paths, run the relevant benchmark before/after.
