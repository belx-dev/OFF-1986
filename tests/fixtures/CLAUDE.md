# tests/fixtures/ — shared test data

27 subfolders of test data spanning proprietary engine formats and standard
formats, consumed by `tests/unit/` and `tests/integration/` (see
`ASSET_SOURCES.md` here for fixture provenance):

- **Engine formats:** `p3d/`, `mlod/`, `rtm/`, `wrp/`, `paa/`, `pac/`, `pbo/`,
  `qstream/`, `savegame/`, `stringtable/` — match the parsers in
  `engine/Poseidon/Asset/Formats/` (see `engine/Poseidon/Asset/CLAUDE.md`).
- **Standard formats:** `audio/` (incl. voice-lang/voice-lang-campaign/wave-to-lip
  subsets), `font/`, `jpg/`, `xml/`.
- **Config:** `cfg/`, `config/`.
- **Missions:** `missions/`, `mission_smoke/`, `mission_linter/` (incl.
  `valid-mission/`, `unused-synthetic/` cases).
- **Mods:** `mods/`, `mods-badmodel/`, `mods-configmerge/`, `mods-manymags/`,
  `mods-mp-missions/`, `workshop/` — each exercises a specific mod-loading
  edge case (bad model, config merge conflicts, large magazine counts,
  MP mission packaging, workshop packaging).
- **Tools:** `evaluator/` (`validate_book/`, `validate_ref/` — SQF doc-example
  validation fixtures), `studio/`.

When adding a fixture, colocate it under the subfolder matching what it
exercises rather than creating a new top-level category — the existing
breadth already covers most format/domain combinations.
