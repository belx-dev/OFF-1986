# engine/Poseidon/

The engine core. It is one CMake target covering all subsystems, so a change in any folder relinks everything that
links `Poseidon`. See `engine/AGENTS.md` for the other engine libraries.

This is a directory map only. It is based on the folder and file names in `ls`, not on reading the code. Per-subsystem
maps do not exist yet. Each subsystem's own `AGENTS.md` is a later task. Read the source before you rely on a guess.

| Dir | What is in it (from names) |
|---|---|
| `AI/` | AI center, groups, subgroups, units, radio, vehicle AI, arcade templates, AI path code |
| `Asset/` | Addon, cache, formats and probe code for game assets |
| `Audio/` | Audio factory, sound scene, speaker, streaming, text, voice, capture and dummy backends |
| `Core/` | Application, config, download, mod system, profiles, progress, server, FSM and game state |
| `Dev/` | Debug, diagnostics and test harness code |
| `Foundation/` | Base layer: algorithms, containers, enums, math, memory, logging, platform, strings, threads, time, types |
| `Game/` | Chat, editor, operation map, mission, scripting, commands and UI action types |
| `Graphics/` | Graphics engine factory and interface, core, rendering, shadows, textures, cursor and dummy backend |
| `Input/` | Keyboard, mouse, gamepad, input subsystem, SDL input, controller UI, combos, cheat codes and viewer controls |
| `IO/` | File server, filesystem, pack files, map files, param files, preprocessor, serialization and streams |
| `Network/` | Network transport (`NetTransport*`), client and server code, master server, network messages, IP bans, XML |
| `Security/` | `Serial.cpp`, `Serial.hpp` and `XOR1024.cpp` (purpose not checked from the source) |
| `UI/` | Display UI, options UI, main menu, multiplayer and mission screens, controls, locale, text and settings |
| `World/` | World object, entities, detection, model, scene, simulation, terrain and world setup |

Files directly under `Poseidon/` include `CMakeLists.txt`, which defines the single `Poseidon` target.
