# engine/Poseidon/Core/ — game loop, FSM, configuration, profiles

## Subfolder map

| Dir | Contents |
|---|---|
| `Config/` | `EngineConfig`, `UserConfig`, `ConfigurationSystem` — unified config from CLI/registry/defaults. |
| `FSM/` | Generic finite-state-machine base class. |
| `Game/` | `GameLoop` — main per-frame loop controller. |
| `Profile/` | `ProfileManager`/`ProfileService` — user profile and difficulty data. |
| `Progress/` | `GlobalAlive` — mission progress tracking (mostly a stub currently). |
| `Server/` | `ServerLoop` — headless server main loop. |

## Key files

- `FSM/Fsm.hpp` — generic FSM: `State` enum, `StateInfo` (name + init/check
  handlers), state variables (`iVar`/`tVar`), timeout support.
- `Config/Configuration.hpp` — `ConfigurationSystem`: merges CLI/registry/defaults,
  language/stringtable init, key-value getters with provenance tracking (`GetSource()`).
- `Config/EngineConfig.hpp` — engine-side config (graphics/input/network/AI/mission settings).
- `Config/UserConfig.hpp` — user/player config (keybindings, video, audio, difficulty).
- `Game/GameLoop.hpp` — main frame loop.
- `Profile/ProfileManager.hpp` — profile lifecycle + persistence.

## Gotchas

- FSM `StateInfo` stores handlers as type-erased `void(*)(void*)`; callers
  cast to the real context type (e.g. `AIGroupContext*`) themselves — there's
  no compile-time type safety here.
- FSM state handlers carry `__attribute__((no_sanitize("function")))` to
  suppress UBSan function-pointer-type checks — this is an intentional,
  ABI-verified workaround, not a bug to "fix."
- Config priority is CLI > registry > defaults;
  `InitializeGameConfiguration()` is the one call that wires up language,
  stringtable, modules, and all config files together — don't duplicate its
  ordering elsewhere.

## Tests

`tests/unit/engine/Poseidon/Core/` — FSM, `EngineConfig`/`UserConfig`
serialization, `ProfileManager`, `GlobalAlive`, perf tracing, plus some
integration-flavored tests (mod install, server mod resolve, window close).
