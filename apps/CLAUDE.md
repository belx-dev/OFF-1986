# apps/

Executable targets and tool entry points. See `apps/README.md` for the
canonical build-target table; this file adds navigation context.

| Dir | Context file | Summary |
|---|---|---|
| `cwr/` | `apps/cwr/CLAUDE.md` | Main game client, demo client, dedicated server — share `GameBase` |
| `fuzzers/` | `apps/fuzzers/CLAUDE.md` | libFuzzer harnesses for asset/protocol parsers |
| `tetris/` | `apps/tetris/CLAUDE.md` | Standalone Tetris sample app (engine smoke test / rendering+input demo) |
| `tools/` | `apps/tools/CLAUDE.md` | Content-pipeline CLIs, ImGui editor (Studio), SQF evaluator REPL, Total Commander plugins, Blender addon |

All GUI/console targets except the Blender addon are C++ and link some subset
of `engine/Poseidon` (+ `PoseidonGL33`/`PoseidonOpenAL` for anything with
graphics/audio). The Blender addon is pure Python and optionally loads
`engine/PoseidonFormats` as a shared library for fast parsing — it has no
CMake involvement, see `apps/tools/BlenderAddon/README.md`.
