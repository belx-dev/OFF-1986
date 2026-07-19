# engine/Poseidon/ — engine core

Single CMake target (`Poseidon.lib`) covering all subsystems below — there is
no per-subsystem static library, so any change here triggers a full relink of
everything that links `Poseidon`. Shares one precompiled header,
`Foundation/PoseidonPCH.hpp` (the `Foundation/PCH/` folder holds the headers
it pulls in: `stdIncludes.h`, `libIncludes.hpp`, `ext_options.hpp`); build with
`POSEIDON_DISABLE_PCH=ON` to audit include self-containment (a manual audit —
this fork has no CI).

14 subsystems, each with its own `CLAUDE.md`:

| Subsystem | Layer | Summary |
|---|---|---|
| `Foundation/` | infra | Containers, strings, memory, threads, math, logging, platform init — replaces STL throughout the engine. Read this first; everything else depends on it. |
| `Core/` | infra | Game loop, FSM, configuration system, user profiles, server loop. |
| `IO/` | infra | ParamFile (config) parsing+preprocessing, binary serialization, filesystem abstraction, streams/compression. |
| `Dev/` | infra | Debug console/HUD, profiling, the in-game test harness server (paired with `engine/Trident`). |
| `World/` | simulation | Entities, terrain, scene/camera, physics/animation/cloth simulation. |
| `Game/` | simulation | Mission loading/lifecycle, SQF scripting bridge (uses `engine/Evaluator`), console command extensions. |
| `AI/` | simulation | Group/subgroup/unit command hierarchy FSMs, vehicle AI, pathfinding (`AI/Path`). |
| `Input/` | simulation | SDL3-backed input capture, action binding, context switching (infantry/vehicle/pilot/viewer). |
| `Asset/` | content | Binary asset loading (P3D models, RTM animations), versioned asset cache, addon/mod registry. |
| `Audio/` | content | Abstract audio engine (backend-pluggable), wave/RIFF parsing, VoN voice chat. |
| `Graphics/` | content | Abstract graphics engine (backend-pluggable), GL state helpers, rendering passes, shadows, textures. |
| `Network/` | content | Multiplayer transport, master-server browser/publisher, auth, IP bans, message codec. |
| `Security/` | content | CD-key validation/encryption. |
| `UI/` | content | In-game menus, mission/campaign editor, options, multiplayer lobby, mod browser, localization/stringtable. |

`Graphics`/`Audio` here define the interface + a `Dummy` no-op backend; real
backends are separate libraries (`engine/PoseidonGL33`, `engine/PoseidonOpenAL`).

## Cross-subsystem conventions

These describe the codebase as inherited. The first three are *inherited
conventions* (see root `CLAUDE.md`, Modernization posture — defaults for
consistency, open to deliberate revision); the last two are closer to
technical invariants until their mechanisms are redesigned.

- Legacy core code uses `AutoArray`, `HashMap`, `RString`, etc.
  (`Foundation/Containers`, `Foundation/Strings`) instead of
  `std::vector`/`std::string` *(inherited convention — and already
  non-uniform: ~270 of ~1180 files here use STL today, mostly newer
  probe/parser/tool-facing code)*.
- `Ref`/`RefCount` (`Foundation/Types/Pointers.hpp`) for ownership instead of
  `std::shared_ptr` *(inherited convention)*.
- Serialization goes through `ParamArchive`/`SerializeBin` (`IO/Serialization`),
  not ad hoc binary I/O *(inherited convention, but save-file compatibility
  makes format changes a versioned, deliberate affair)*.
- `USE_CASTING`/`DEFINE_CASTING` macros implement RTTI-free `dyn_cast` for the
  `World/Scene/Object` entity hierarchy (the build compiles without RTTI
  assumptions in this hierarchy — replacing this means a coordinated sweep).
- Most data structures are explicitly **not thread-safe** — callers serialize
  access; exceptions are noted per-subsystem (e.g. `Stringtable`'s Meyer's singleton).
