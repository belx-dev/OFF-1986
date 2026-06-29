# engine/Poseidon/Game/ — missions and scripting

## Subfolder map

| Dir | Contents |
|---|---|
| `Commands/` | Game-state command extensions (`GameStateExt*.cpp` — test/audio/UI/world/waypoint variants). |
| `Mission/` | Mission file loading (`MissionInfo.hpp`, `MissionPathLoader.hpp`). |
| `Scripting/` | Mission script interpreter (`Scripts.hpp`, `ExpressExt.cpp`), built on `engine/Evaluator/express.hpp`. |

## Key files

- `Scripting/Scripts.hpp` — `Script` class: `GameValue`-based scripting,
  line/label/statement parsing, the `SimulateBody()` execution loop, variable
  namespace (`GameVarSpace`).
- `Mission/MissionInfo.hpp` — `IsActive()`, `AvailableEndings()` — free
  functions wrapping the `GWorld` global; mission introspection API.
- `Commands/GameStateExt.hpp` — `GameData` subclasses bridging engine types
  into `GameValue` (`GameObject`, `GameVector`, `GameTrans`, `GameOrient`,
  `GameSide`, `GameGroup`, `GameFile`) — this is the scripting-engine bridge.
- `Editor.hpp`/`.cpp` — in-mission editor state.
- `OperMap.hpp` + `OperMapDoor.cpp` — tactical map with interactable doors.

## Gotchas

- SQF scripts are built on `engine/Evaluator/express.hpp` — read that file's
  `CLAUDE.md` before touching the scripting bridge here.
- `ScriptLine` kinds: `waitUntil` (condition-gated), `suspendUntil`
  (time-based pause), `condition` (skip if false), `statement` (code) — know
  which kind you're modifying, they have different scheduling semantics.
- `GameValue` dispatches by tagged type ranges (`0x100`=`GameObject`,
  `0x200`=`GameVector`, `0x400`=`GameTrans`, ...) — adding a new `GameValue`
  type means picking an unused range, not just adding an enum value.
- `MissionInfo::IsActive()` returns `false` in the main menu even though
  `GWorld` exists (there's no real player yet) — don't assume `GWorld != null`
  means a mission is running.
- Mission endings are sensors named `ASTEnd1`..`ASTEnd6`; `"win"`/`"lose"`/`"killed"`
  are always available regardless of mission script content.

## Tests

`tests/unit/engine/Poseidon/Game/` — game-state extension tests, title
effects, tristate value getters, mission loading.
