# apps/cwr/ — game client, demo client, dedicated server

Three executables sharing one boot/lifecycle library:

| Dir | Target | Type | Purpose |
|---|---|---|---|
| `GameBase/` | `GameBase` (static lib) | — | Shared boot code: memory system, config parsing, file-bank/engine-core init, FPU setup. Defines pure-virtual hooks (`ConfigureBankMerge`, `LogDomain`, etc.) that subclasses must implement. |
| `Game/` | `PoseidonGame` | GUI exe | Full game client. |
| `GameDemo/` | `PoseidonGameDemo` | GUI exe | Feature-restricted demo client (Single Missions + Editor only). |
| `Server/` | `PoseidonServer` | console exe | Headless dedicated multiplayer server. |

## Key files

- `Game/WinMain.cpp` — platform entry point, constructs `GameApplication`.
- `Game/GameApplication.cpp`/`.hpp` — main client app logic: graphics engine
  creation, content reload, mod mounting, sound/input init, world init.
  Inherits `GameBase`.
- `GameDemo/GameDemoApplication.cpp` — subclasses `GameApplication`; reuses
  `GameApplication.cpp`/`HighPerformanceGpuHint.cpp` as **compiled sources**
  (not a linked library) so demo feature gates are baked in at compile time.
- `Server/ServerMain.cpp` + `ServerApplication.cpp` — headless variant, no
  graphics backend linked.
- `GameBase/GameBase.cpp`/`.hpp` — shared lifecycle base class.

## Gotchas

- `GameApplication::InitializeWorld()` and `RegisterGraphicsBackends()` are
  virtual and overridden by other apps (e.g. `apps/tetris`) to skip AI/vehicle
  simulation entirely — check the override before assuming full-engine boot.
- Windows links `PoseidonGame`/`PoseidonGameDemo`/`PoseidonServer` with
  `/SAFESEH:NO` for legacy external object-file compatibility *(inherited
  constraint — worth revisiting during modernization: once no non-SafeSEH
  object files remain in the link, the flag can and should go)*.
- Demo and Tetris both compile `GameApplication.cpp` directly rather than
  linking against `Game` — if you change `GameApplication.cpp`, check all
  in-tree includers, not just `Game`'s own CMake target.
