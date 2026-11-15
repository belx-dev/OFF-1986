#pragma once

#include <cstdint>

// On-disk layout of a render-frame dump (`--render-frame-dump <path>`).
//
// A `Frame` references meshes by backend handle only (`MeshHandle.vao` is a
// GL object id), so a Frame alone cannot be drawn by another graphics API.
// The dump adds what the Frame lacks: the bytes of every vertex/index
// buffer a draw references plus the VAO's attribute layout.  Written by
// `Engine::DumpFrameResources` (GL33: `EngineGL33_FrameDump.cpp`), read by
// `apps/tools/VulkanSpike`.
//
// Self-contained on purpose: no Poseidon includes, so a standalone sample
// can read a dump without linking the engine.  All integers little-endian,
// all structs written with their natural layout (static_asserts below pin
// the sizes).  Enum values use the GL enum numbers so the reader needs no
// Poseidon headers (GL_FLOAT = 0x1406, GL_UNSIGNED_BYTE = 0x1401, ...).
//
// File = DumpHeader, then `meshCount` x (DumpMesh, vertexBytes, indexBytes),
// then `drawCount` x DumpDraw.

namespace Poseidon::render::frame::dump
{

constexpr std::uint32_t kMagic = 0x4446464Fu; // "OFFD" little-endian
constexpr std::uint32_t kVersion = 1;
constexpr std::uint32_t kAttribCount = 4;

struct DumpHeader
{
    std::uint32_t magic = kMagic;
    std::uint32_t version = kVersion;
    std::uint32_t meshCount = 0;
    std::uint32_t drawCount = 0;
    float view[16] = {};       // GfxMatrix bytes as uploaded to the GL UBO
    float projection[16] = {}; // GL33 clip space: y up, depth 0..1 (glClipControl ZERO_TO_ONE)
    std::int32_t viewport[4] = {};
    float nearPlane = 0.0f;
    float farPlane = 0.0f;
};
static_assert(sizeof(DumpHeader) == 16 + 128 + 16 + 8);

struct DumpAttrib
{
    std::uint32_t enabled = 0;
    std::uint32_t components = 0;
    std::uint32_t type = 0; // GL enum
    std::uint32_t normalized = 0;
    std::uint32_t offset = 0; // bytes from vertex start
};
static_assert(sizeof(DumpAttrib) == 20);

struct DumpMesh
{
    std::uint32_t vao = 0; // handle the draws reference
    std::uint32_t stride = 0;
    DumpAttrib attribs[kAttribCount] = {}; // 0 pos, 1 normal, 2 uv, 3 landClip (SVertex)
    std::uint32_t vertexBytes = 0;         // followed by this many bytes
    std::uint32_t indexBytes = 0;          // then this many bytes (uint16 indices)
};
static_assert(sizeof(DumpMesh) == 8 + 4 * 20 + 8);

// Pass kinds: FramePassKind values (Frame.hpp).  Descriptor fields: the
// RenderPassDescriptor enum values (RenderPassDescriptor.hpp).
struct DumpDraw
{
    std::uint32_t passKind = 0;
    std::uint32_t vao = 0;
    std::uint32_t indexBegin = 0; // first index (not bytes)
    std::uint32_t indexCount = 0;
    std::uint32_t cull = 0;
    std::uint32_t blend = 0;
    std::uint32_t depth = 0;
    std::uint32_t alpha = 0;
    float world[16] = {}; // camera-relative, GfxMatrix bytes
};
static_assert(sizeof(DumpDraw) == 32 + 64);

} // namespace Poseidon::render::frame::dump
