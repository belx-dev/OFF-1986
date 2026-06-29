# engine/Poseidon/Graphics/ — abstract graphics engine

Backend-pluggable: this subsystem defines the interface + state model + a
`Dummy` headless backend; the real implementation is `engine/PoseidonGL33`.

## Subfolder map

| Dir | Contents |
|---|---|
| `Core/` | GL state helpers — blend/cull/depth-stencil state, samplers, index buffers, pipeline state, matrix conversion, fan decomposition. |
| `Cursor/` | Cursor rendering. |
| `Dummy/` | Headless/server backend. |
| `Rendering/` | High-level frame/pass management. |
| `Rendering/Draw/` | Draw dispatch. |
| `Rendering/Effects/` | Shader effects. |
| `Rendering/Font/` | Text rendering. |
| `Rendering/Frame/` | Frame structure, render passes. |
| `Rendering/Lighting/` | Lights, materials, transitions. |
| `Rendering/Primitives/` | Geometry primitives (boxes, spheres, etc.). |
| `Rendering/Shape/` | Shape rendering. |
| `Shadow/` | Shadow mapping. |
| `Shared/` | Screenshot writers (BMP/PNG/TGA), RenderDoc capture hookup, window metrics. |
| `Textures/` | Texture management. |

## Key files

- `GraphicsEngineFactory.hpp` — backend enum (`Dummy=0`, `GL33=33`, `Auto`), param struct, creation result — how `PoseidonGL33` plugs in.
- `Core/GLPipelineState.hpp` — named helpers (`SetColorMask`,
  `SetPolygonOffsetForDecals`, depth-clamp toggles); **state-machine
  pattern** — `ApplyPipeline` sets ALL pipeline state in one place, no raw
  `glEnable` calls scattered through the backend.
- `Core/GLBlendState.hpp` — blend mode state.
- `Rendering/Lighting/Material.hpp` — material properties.
- `Shared/WindowMode.hpp` — window/fullscreen modes.

## Gotchas

- Decal polygon-offset defaults to `(-1, -1)` for coplanar surface
  disambiguation — don't "fix" this thinking it's a stray sign error.
- Any new GL state must go through `GLPipelineState`'s `ApplyPipeline`
  pattern, not a direct `glEnable`/`glDisable` call in `PoseidonGL33` — keeps
  state changes auditable/cacheable.
- `Dummy` backend exists specifically so `apps/cwr/Server` can run with zero
  graphics dependency — don't add code paths here that assume a real GL context exists.

## Tests

`tests/unit/engine/Poseidon/Graphics/` — 30+ tests: GL state caching,
ownership audits, buffer usage, fan decompose, image import (JPG/DDS), mipmap
layout, alpha classification, DXT compression, IBO bind audits, end-to-end
rendering (`test_gl33_rendering.cpp`).
