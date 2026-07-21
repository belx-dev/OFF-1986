# apps/tetris/Tetris/ — sample game / rendering+input benchmark

`PoseidonTetris` (GUI exe). Standalone Tetris game built on the same engine
boot path as the real client, used as a lightweight demo and as a way to
exercise rendering/input without full world/AI/vehicle simulation overhead.

- `WinMain.cpp` — entry point, constructs `TetrisApplication`.
- `TetrisApplication.cpp`/`.hpp` — subclasses `GameApplication`
  (`apps/cwr/Game/GameApplication.cpp`, compiled in-tree, not linked), but
  overrides `InitializeWorld()`/`RegisterGraphicsBackends()` to skip
  AI/vehicle/world init and boot only the rendering engine.
- `TetrisGame.cpp`/`.hpp` — piece spawn/rotate/collision/scoring logic.
- `TetrisNotebookUI.cpp` + `NotebookScene.cpp` — ImGui notebook-style overlay UI
  and its 3D scene.

Links `Poseidon`, `PoseidonGL33`, `PoseidonOpenAL`, `GameBase` — same backends
as the real game, just a much narrower runtime surface (aggressive
`/OPT:REF /OPT:ICF` + function-level COMDATs drop unused engine code from the
final binary).
