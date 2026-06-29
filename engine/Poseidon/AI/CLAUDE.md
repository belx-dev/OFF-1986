# engine/Poseidon/AI/ — decision-making and pathfinding

3-tier command hierarchy: `AICenter` (global) → `AIGroup` → `AISubgroup` → `AIUnit`.

## Subfolder map

| Dir | Contents |
|---|---|
| `Path/` | Pathfinding: A* search, formation steering (`AStar.hpp`, `PathPlanner.hpp`, `PathSteer.hpp`). |
| (root) | Group/subgroup/unit AI, vehicle AI, target/visibility tracking. |

## Key files

- `EntityAI.hpp` — `VisibilityTracker`/`VisibilityTrackerCache` (LOS caching),
  `Target`/`TargetList`, `TargetType` hierarchy.
- `Path/PathPlanner.hpp` — `AI` base class (extends `NetworkObject`),
  `Semaphore`/`Formation`/`FormationPos` enums, `Answer` enum (bit-offset
  hierarchy: unit→subgroup at `0x000`, subgroup→group at `0x100`,
  group→center at `0x200`).
- `AIGroup.hpp`/`.cpp` (+ `AIGroupImpl*.cpp`, `AIGroupCmd.cpp`) — group command interface.
- `AISubgroup.cpp` + `AISubgroupFSM.cpp` (+ `AISubgroupFSMSupply.inc`) — tactical FSM, including supply/ammo resupply logic.
- `VehicleAI.hpp` + `VehicleAICombat.cpp`/`VehicleAIPilot.cpp` — vehicle-specific behavior (ground combat vs. air/pilot).

## Gotchas

- `Formation`/`Semaphore` are stored as plain `int`, with `-1` meaning
  "not set" in save files — don't assume a valid enum value is always present
  when deserializing.
- `Answer` bit-range offsets (`0x000`/`0x100`/`0x200`) encode which tier of
  the hierarchy a result came from — mask before comparing, don't compare raw values across tiers.
- `VisibilityTracker` caches last LOS calc time + result; `KnownValue()`
  checks cache freshness before recomputing — if you add a new visibility
  query, decide whether it should go through the cache or bypass it.
- `PathPlanner` uses sentinel float values for unreachable cells:
  `GET_UNACCESSIBLE` (1e20) vs `SET_UNACCESSIBLE` (1e30) — these are distinct
  sentinels, not interchangeable.

## Tests

`tests/unit/engine/Poseidon/AI/` — AI basics, subgroup FSM, unit display
names, event handlers, pathfinding, vehicle AI, entity language-switching.
