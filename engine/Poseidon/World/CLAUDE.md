# engine/Poseidon/World/ — entities, terrain, scene, simulation

## Subfolder map

| Dir | Contents |
|---|---|
| `Detection/` | Line-of-sight / visibility calculation (`Detector.hpp`). |
| `Entities/Infantry/` | Soldier/person entities, movement, actions (`Person.hpp`, `SoldierOld.hpp`). |
| `Entities/Vehicles/{Air,Ground,Misc}` | Vehicle entity hierarchy, base `Entity` class (`Vehicle.hpp`). |
| `Entities/Weapons/` | Projectiles, weapon fire (`Weapons.hpp`). |
| `Model/` | 3D model loading, LOD, shape management (`Model.hpp`, `ModelCache.hpp`). |
| `Scene/`, `Scene/Camera/` | `Object` base class, camera system, rendering sort lists (`Object.hpp`, `Scene.hpp`). |
| `Simulation/{Animation,Cloth}/` | Animation playback, cloth simulation, collision. |
| `Terrain/` | Heightmap, `.wrp` terrain file reading, roads, occlusion/visibility (`Landscape.hpp`, `WrpReader.hpp`). |

## Key files

- `Scene/Object.hpp` — base `Object` class: serialization, damage regions,
  targeting enums (`TargetSide`), `ObjectType`/`DestructType`. **Every game
  object inherits from this.**
- `Entities/Vehicles/Vehicle.hpp` — `Entity` (extends `Object`):
  `SimulationImportance` enum, `EntityType` registry, fire results, condition levels.
- `World.hpp` — `World` master class: camera types, `VehicleList`,
  `PreloadedVType` enum, mission state.
- `Terrain/Landscape.hpp` — `GLandscape` global, `LandRange`/`TerrainRange`
  macros, grid-based spatial indexing (`InRange` macro).
- `Scene/Scene.hpp` — FOV/fog macros, `SortObject` rendering sort lists.

## Gotchas

- Entity identity/casting uses `USE_CASTING`/`DEFINE_CASTING` macros for an
  RTTI-free `dyn_cast`, not `dynamic_cast`.
- Serialization goes through `ParamArchive` with version branches (see
  `UnitStatusBase`) — adding fields needs a version bump and a branch, not a
  silent layout change.
- Grid math (`InRange`, `TerrainInRange`) assumes `LandRange` is a power of
  two and relies on that for bit-logic shortcuts — don't change grid sizing
  without checking these macros.
- `SimulationImportance` drives LOD-based simulation culling
  (`SimulateCamera` → `SimulateVisibleNear` → ... → `InvisibleFar`); entities
  far from any camera get cheaper updates.
- `TargetSide` includes both real sides (`TEast`/`TWest`/`TGuerrila`/`TCivilian`)
  and pseudo-states (`TEnemy`/`TFriendly`/`TLogic`) — don't conflate the two
  when comparing sides.

## Tests

`tests/unit/engine/Poseidon/World/` — terrain loading, occlusion, segment-cache
LOD, gamestate serialization, sky preload audit.
