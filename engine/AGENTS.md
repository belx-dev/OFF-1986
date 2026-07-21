# engine/

Static C++ libraries forming the engine stack, plus one Rust crate. See
`engine/README.md` for the canonical build-output table.

```
Poseidon (+ PoseidonGL33, PoseidonOpenAL)  ← full game engine
```

| Dir | Output | Context file | Summary |
|---|---|---|---|
| `Poseidon/` | `Poseidon.lib` | `engine/Poseidon/AGENTS.md` | The engine core — huge, has its own per-subsystem AGENTS.md files. |
| `PoseidonGL33/` | `PoseidonGL33.lib` | `engine/PoseidonGL33/AGENTS.md` | OpenGL 3.3 rendering backend, implements `Poseidon`'s abstract graphics interface. |
| `PoseidonOpenAL/` | `PoseidonOpenAL.lib` | `engine/PoseidonOpenAL/AGENTS.md` | OpenAL audio backend (sound + voice-over-network). |
| `PoseidonFormats/` | `.dll`/`.lib` | `engine/PoseidonFormats/AGENTS.md` | C API for reading P3D/PAA/PBO/RTM, used by the Blender addon and TC plugins. |
| `Evaluator/` | header+source | `engine/Evaluator/AGENTS.md` | SQF expression evaluator core (parser + `GameValue`/`GameState`); consumed by `Poseidon/Game/Scripting`. |
| `Random/` | header-mostly | `engine/Random/AGENTS.md` | Deterministic seeded PRNG for world/NPC generation. |
| `Trident/` | `tri` (Rust bin) | `engine/Trident/AGENTS.md` | Test orchestrator — spawns game binaries with `--harness <port>`, drives SQF-scripted scenarios over TCP. Build separately with Cargo, not CMake. |

`Poseidon` is built as a single CMakeLists.txt target covering all its
subsystems — there's no per-subsystem static library, so changes anywhere in
`Poseidon/` trigger a full relink of everything that links it.
