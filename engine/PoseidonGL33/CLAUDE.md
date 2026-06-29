# engine/PoseidonGL33/ — OpenGL 3.3 rendering backend

Implements the abstract graphics `Engine` interface declared in
`engine/Poseidon/Graphics/` (see `engine/Poseidon/Graphics/CLAUDE.md`) with a
concrete GL33 + SDL3 backend. Selected via the engine's `GraphicsEngineFactory`
(backend enum includes `Dummy`, `GL33`, `Auto`) at runtime/link time.

## Key files

- `EngineGL33.hpp` — main renderer interface/state.
- `EngineGL33*.cpp` — renderer state, shader pipelines, shadows, 2D/3D draw paths, lifecycle.
- `TextureGL33.hpp` + `TextureBankGL33_*.cpp` — texture loading/caching.
- `SDLEventWindow.hpp` — SDL window + input event abstraction.
- `GLVertexAttribLayouts.hpp` — vertex format definitions.

Depends on `Poseidon` and `OpenGL::GL`, with an embedded GLAD loader
(see `thirdparty/glad`). Linked by GUI targets that request the GL33 backend
(`apps/cwr/Game`, `GameDemo`, `apps/tetris`, `apps/tools/Studio`/`Tools`).
`apps/cwr/Server` does not link this — it runs headless against the `Dummy`
graphics backend in `engine/Poseidon/Graphics/Dummy`.
