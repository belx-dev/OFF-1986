# engine/Evaluator/ — SQF expression evaluator core

The parser/evaluator for SQF, the engine's mission scripting language. No
CMake target of its own (engine-internal, header+source consumed directly by
`Poseidon`); the standalone CLI wrapping it lives at `apps/tools/Evaluator/Cli`
(`PoseidonEvaluator` target).

## Key files

- `express.hpp` — core types: `GameState`, `GameValue`, operators (~30KB header).
- `express.cpp` — parser + evaluator implementation (~90KB).
- `EvalState.hpp`/`.cpp` — evaluator state container.
- `EvaluatorHost.hpp`/`.cpp` + `EvaluatorRuntimeAnchor.cpp`/`EvaluatorRuntimeStubs.cpp` —
  host embedding glue and runtime stubs for standalone (non-engine) use.
- `MockObjects.hpp` — test doubles for game entities, used in unit tests.
- `Validate.hpp`/`.cpp` — validates book/reference-doc SQF examples against the real evaluator.
- `SqsRunner.hpp`/`.cpp` — runs `.sqs` script tests.

Consumed by `engine/Poseidon/Game/Scripting/` (the in-engine scripting bridge —
see `engine/Poseidon/Game/CLAUDE.md`), the `PoseidonEvaluator` CLI tool, and
indirectly by `engine/Trident` test scenarios (via the in-game harness, not a
direct dependency).
