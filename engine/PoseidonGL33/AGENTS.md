# engine/PoseidonGL33/ — OpenGL 3.3 rendering backend

Implements the abstract graphics `Engine` interface declared in
`engine/Poseidon/Graphics/` (see `engine/Poseidon/Graphics/AGENTS.md`) with a
concrete GL33 + SDL3 backend. Selected via the engine's `GraphicsEngineFactory`
(backend enum includes `Dummy`, `GL33`, `Auto`) at runtime/link time.

## Key files

- `EngineGL33.hpp` — main renderer interface/state.
- `EngineGL33*.cpp` — renderer state, shader pipelines, shadows, 2D/3D draw paths, lifecycle.
- `TextureGL33.hpp` + `TextureBankGL33_*.cpp` — texture loading/caching.
- `SDLEventWindow.hpp` — SDL window + input event abstraction.
- `GLVertexAttribLayouts.hpp` — vertex format definitions.
- `EngineGL33_FrameDump.cpp` — `--render-frame-dump`: serialises a built `Frame` plus the VAO/VBO/IBO
  contents it references (format: `Poseidon/Graphics/Rendering/Frame/FrameDumpFormat.hpp`) for the Vulkan spike.
  `_drawItems` (the per-frame draw record the frame observer reads) is cleared only in `InitDraw`; `BeginPass`
  must not clear it or a 3D → screen-space → 3D frame loses its first segment.

Depends on `Poseidon` and `OpenGL::GL`, with an embedded GLAD loader
(see `thirdparty/glad`). Linked by GUI targets that request the GL33 backend
(`apps/cwr/Game`, `GameDemo`, `apps/tetris`). `apps/tools/Studio`/`Tools` use
SDL3 directly for their preview windows and do not link this.
`apps/cwr/Server` does not link this either — it runs headless against the
`Dummy` graphics backend in `engine/Poseidon/Graphics/Dummy`.
