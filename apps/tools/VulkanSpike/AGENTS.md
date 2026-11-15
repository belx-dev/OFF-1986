# apps/tools/VulkanSpike/ — Frame-on-Vulkan spike

Standalone offscreen renderer for issue off-1986-project#21: reads a frame
dump written by the game (`--render-frame-dump <path>`, format in
`engine/Poseidon/Graphics/Rendering/Frame/FrameDumpFormat.hpp`) and draws its
opaque geometry with direct Vulkan 1.3 and a flat shader, then writes a PPM.
Findings and the recommendation live in
`docs/architecture/spikes/2026-11-vulkan-frame.md` (project repo).

- Not part of the default build: `cmake --preset linux-x64-clang-rwdi-vulkan`
  (adds `POSEIDON_BUILD_VULKAN_SPIKE=ON` and the vcpkg feature `vulkan-spike`).
- Links the system Vulkan loader and vcpkg `vulkan-headers`; does not link
  the engine.
- `main.cpp` is one file on purpose: it is the "how much code does direct
  Vulkan cost" measurement.
- Usage: `PoseidonVulkanSpike <dump> <out.ppm> [--validate] [--gpu N]`.
  Exit 0 = image written and at least 1% of pixels covered by geometry.
