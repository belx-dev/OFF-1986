# tests/unit/ — Catch2 C++ unit tests

Mirrors the `engine/`+`apps/` source tree path-for-path — to find tests for
`engine/Poseidon/Foo/Bar.cpp`, look in `tests/unit/engine/Poseidon/Foo/`. Test
files are named `test_*.cpp`.

| Suite | Location | Covers |
|---|---|---|
| PoseidonFoundationTests | `engine/Poseidon/Foundation/` | Low-level containers, strings, memory, threads, IO/runtime glue |
| PoseidonCoreTests | `engine/Poseidon/` | Evaluator, config/ParamFile, QStream, locale/stringtable, preproc, CSV |
| PoseidonTests | `engine/Poseidon/` | P3D formats, graphics, audio, core, AI, entities |
| PoseidonTests `[config]` | `engine/Poseidon/Core/` | Config store, profiles |
| PoseidonTests `[graphics]` | `engine/Poseidon/Graphics/` | Graphics + shader compilation |
| PoseidonServerTests | `apps/Server/` | Server simulate mode |
| PoseidonEvaluatorTests | `apps/Evaluator/` | SQF evaluation + evaluator tooling |

Linked against `Catch2::Catch2WithMain`; fixtures are copied at build time
from `tests/fixtures/` into the test binary's directory. Tests requiring real
game data are tagged (`[GameData]`/`[gamescan]`/`[external-data]`) and skip
when the data directory isn't present.

Windows CTest registration goes through `cmake/CatchWindowsSafe.cmake` /
`CatchAddWindowsSafeTests.cmake` to sanitize test names with special
characters — don't register Catch2 tests directly with
`catch_discover_tests()` for new targets here without checking those helpers first.

When adding a source file under `engine/` or `apps/`, mirror its path under
`tests/unit/` for the corresponding test file — this convention is what makes
the tree navigable.
