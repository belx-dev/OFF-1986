# engine/Poseidon/Input/ — input mapping and context

Flat directory (no subfolders). SDL3-backed input capture with context-aware
action binding (different action sets for infantry/vehicle/pilot/viewer).

## Key files

- `InputSubsystem.hpp` — singleton; per-frame `Update()`, context switching
  (`SetContext`), action queries (`GetAction`), computed movement state
  (`moveForward_`, `turnLeft_`, `fire_`), activity tracking
  (`MarkKeyboardMoveActive`, `IsJoystickActive`).
- `InputProcessingSdl.cpp` — SDL3 keyboard/mouse/gamepad capture (`InputCode.hpp`).
- `InputProfile.hpp` — action → `InputCode` binding storage + serialization.
- `ControlsCategory.hpp` — action grouping (infantry, vehicle, viewer, pilot).
- `InputContext.hpp` — context enum that switches the active action set.
- `KeyboardState.hpp` / `MouseState.hpp` — per-frame device state; mouse has acceleration curves (`ResponseCurve.hpp`).

## Gotchas

- `GetAction()` returns a **float** (analog axis 0.0–1.0), even for digital
  buttons — don't treat it as a bool without thresholding.
- `GetActionToDo()` fires once per press when `reset=true` — using the plain
  `GetAction()` for a "did the player just press X" check will fire every frame the key is held.
- `InputContext` must be switched explicitly (`SetContext()`) when the player
  changes infantry/vehicle/pilot/viewer state — bindings don't auto-resolve by entity type.
- Activity timestamps (keyboard vs. gamepad vs. mouse) drive which control
  hints the UI shows — update them if you add a new input source.

## Tests

`tests/unit/engine/Poseidon/Input/` — binding conflicts, combo detection,
controller UI layout, keyboard/mouse/gamepad state, response curves, rumble
manager, key repeat. Extensive coverage relative to subsystem size — check
here before assuming a binding edge case is untested.
