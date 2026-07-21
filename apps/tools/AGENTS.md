# apps/tools/ — content pipeline, editor, and plugins

| Dir | Target | Type | Purpose |
|---|---|---|---|
| `Tools/` | `PoseidonTools` | console exe | Swiss-army CLI: model/image/PBO/terrain/font/sound/config/stringtable/SQF-lint/scan/mine/input/von/shadow commands (`Tools/commands/*.cpp`, one file per command, dispatched via CLI11 in `Tools/main.cpp`). `SDLPreview.cpp` provides an SDL3 preview window for asset inspection. |
| `Evaluator/Cli/` | `PoseidonEvaluator` | console exe | Standalone SQF REPL — `Cli/main.cpp` drives the engine's `engine/Evaluator` scripting evaluator in isolation, no graphics/audio. |
| `Studio/` | `PoseidonStudio` | GUI exe | ImGui+SDL3 interactive asset/editor shell (`StudioApp.cpp`). Links full graphics+audio backends plus FreeType for text. |
| `TcPbo/` | `TcPbo` (module, output name `pbo`/.wcx) | plugin | Total Commander packer plugin for browsing/extracting PBO archives (`tc_pbo.cpp`, `wcxhead.h`). |
| `TcLister/` | `TcLister` (module, output name `poseidon`/.wlx) | plugin | Total Commander lister plugin for previewing engine asset files inline (`tc_lister.cpp`, `listplug.h`). |
| `BlenderAddon/io_import_p3d/` | — | Python | Blender 4.x addon importing P3D models. `import_p3d.py` (mesh/material/armature build), `p3d_lib.py` (binary decoder), `operators.py`/`panels.py` (UI). Optionally backed by the compiled `PoseidonFormats` shared lib for performance. No CMake; see its own `README.md` for packaging (`uv` for Python deps). |

Tool targets link only the engine libraries their command surface needs
(narrower than the GUI game targets in `apps/cwr/`); check each target's
`CMakeLists.txt` before assuming a dependency is available.
