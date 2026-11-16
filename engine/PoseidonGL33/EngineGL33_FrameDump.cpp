#include <PoseidonGL33/EngineGL33.hpp>
#include <PoseidonGL33/GL33BindCache.hpp>
#include <Poseidon/Graphics/Rendering/Frame/Frame.hpp>
#include <Poseidon/Graphics/Rendering/Frame/FrameDumpFormat.hpp>

#include <glad/gl.h>

#include <cstdio>
#include <cstring>
#include <unordered_set>
#include <vector>

// Serialise a built Frame plus the GL buffers its draws reference.  Reads
// the VAO state and buffer contents back through plain GL queries, so it
// does not need the backend's mesh bookkeeping.  Diagnostics only: stalls
// the pipeline (glGetBufferSubData) and is meant to run once.

namespace
{

using namespace Poseidon::render::frame;

bool writeAll(FILE* f, const void* p, size_t n)
{
    return n == 0 || fwrite(p, 1, n, f) == n;
}

void copyMatrix(float out[16], const Poseidon::GfxMatrix& m)
{
    memcpy(out, &m, sizeof(float) * 16);
}

// Describe the currently bound VAO and read its vertex (attrib 0's buffer)
// and index buffers.  Returns false if the VAO references no buffers.
bool readMesh(unsigned int vao, dump::DumpMesh& mesh, std::vector<unsigned char>& vertices,
              std::vector<unsigned char>& indices)
{
    GL33Bind::Vao(vao);
    mesh = {};
    mesh.vao = vao;

    GLint vbo = 0;
    for (std::uint32_t i = 0; i < dump::kAttribCount; ++i)
    {
        GLint enabled = 0, size = 0, type = 0, normalized = 0, stride = 0, buffer = 0;
        void* pointer = nullptr;
        glGetVertexAttribiv(i, GL_VERTEX_ATTRIB_ARRAY_ENABLED, &enabled);
        if (!enabled)
            continue;
        glGetVertexAttribiv(i, GL_VERTEX_ATTRIB_ARRAY_SIZE, &size);
        glGetVertexAttribiv(i, GL_VERTEX_ATTRIB_ARRAY_TYPE, &type);
        glGetVertexAttribiv(i, GL_VERTEX_ATTRIB_ARRAY_NORMALIZED, &normalized);
        glGetVertexAttribiv(i, GL_VERTEX_ATTRIB_ARRAY_STRIDE, &stride);
        glGetVertexAttribiv(i, GL_VERTEX_ATTRIB_ARRAY_BUFFER_BINDING, &buffer);
        glGetVertexAttribPointerv(i, GL_VERTEX_ATTRIB_ARRAY_POINTER, &pointer);
        auto& a = mesh.attribs[i];
        a.enabled = 1;
        a.components = static_cast<std::uint32_t>(size);
        a.type = static_cast<std::uint32_t>(type);
        a.normalized = static_cast<std::uint32_t>(normalized);
        a.offset = static_cast<std::uint32_t>(reinterpret_cast<std::uintptr_t>(pointer));
        if (i == 0)
        {
            vbo = buffer;
            mesh.stride = static_cast<std::uint32_t>(stride);
        }
    }

    GLint ibo = 0;
    glGetIntegerv(GL_ELEMENT_ARRAY_BUFFER_BINDING, &ibo);
    if (vbo == 0 || ibo == 0)
        return false;

    GLint vbytes = 0, ibytes = 0;
    glBindBuffer(GL_ARRAY_BUFFER, static_cast<GLuint>(vbo));
    glGetBufferParameteriv(GL_ARRAY_BUFFER, GL_BUFFER_SIZE, &vbytes);
    glGetBufferParameteriv(GL_ELEMENT_ARRAY_BUFFER, GL_BUFFER_SIZE, &ibytes);
    if (vbytes <= 0 || ibytes <= 0)
        return false;

    vertices.resize(static_cast<size_t>(vbytes));
    indices.resize(static_cast<size_t>(ibytes));
    glGetBufferSubData(GL_ARRAY_BUFFER, 0, vbytes, vertices.data());
    glGetBufferSubData(GL_ELEMENT_ARRAY_BUFFER, 0, ibytes, indices.data());
    mesh.vertexBytes = static_cast<std::uint32_t>(vbytes);
    mesh.indexBytes = static_cast<std::uint32_t>(ibytes);
    return true;
}

} // namespace

bool EngineGL33::DumpFrameResources(const Poseidon::render::frame::Frame& frame, const char* path)
{
    if (!_glContext || !path)
        return false;

    // Collect draws and the distinct meshes they reference.
    std::vector<dump::DumpDraw> draws;
    std::vector<unsigned int> vaos;
    std::unordered_set<unsigned int> seen;
    for (const Pass& p : frame.passes)
    {
        for (const Draw& d : p.draws)
        {
            if (d.mesh.vao == 0 || d.indexCount <= 0)
                continue;
            dump::DumpDraw out;
            out.passKind = static_cast<std::uint32_t>(p.kind);
            out.vao = d.mesh.vao;
            out.indexBegin = static_cast<std::uint32_t>(d.indexBegin);
            out.indexCount = static_cast<std::uint32_t>(d.indexCount);
            out.cull = static_cast<std::uint32_t>(d.descriptor.cull);
            out.blend = static_cast<std::uint32_t>(d.descriptor.blend);
            out.depth = static_cast<std::uint32_t>(d.descriptor.depth);
            out.alpha = static_cast<std::uint32_t>(d.descriptor.alpha);
            copyMatrix(out.world, d.world);
            draws.push_back(out);
            if (seen.insert(d.mesh.vao).second)
                vaos.push_back(d.mesh.vao);
        }
    }

    FILE* f = fopen(path, "wb");
    if (!f)
        return false;

    dump::DumpHeader h;
    copyMatrix(h.view, frame.camera.view);
    copyMatrix(h.projection, frame.camera.projection);
    h.viewport[0] = frame.camera.viewport.x;
    h.viewport[1] = frame.camera.viewport.y;
    h.viewport[2] = frame.camera.viewport.width;
    h.viewport[3] = frame.camera.viewport.height;
    h.nearPlane = frame.camera.nearPlane;
    h.farPlane = frame.camera.farPlane;
    h.drawCount = static_cast<std::uint32_t>(draws.size());

    // Meshes are written as they are read; count the ones that had buffers.
    std::vector<unsigned char> vertices, indices;
    bool ok = true;
    long headerPos = ftell(f);
    ok = ok && writeAll(f, &h, sizeof(h));
    for (unsigned int vao : vaos)
    {
        dump::DumpMesh m;
        if (!readMesh(vao, m, vertices, indices))
            continue;
        ok = ok && writeAll(f, &m, sizeof(m));
        ok = ok && writeAll(f, vertices.data(), vertices.size());
        ok = ok && writeAll(f, indices.data(), indices.size());
        ++h.meshCount;
    }
    ok = ok && writeAll(f, draws.data(), draws.size() * sizeof(dump::DumpDraw));
    // Patch the header now that meshCount is known.
    ok = ok && fseek(f, headerPos, SEEK_SET) == 0 && writeAll(f, &h, sizeof(h));
    ok = fclose(f) == 0 && ok;

    // Reading back touched GL_ARRAY_BUFFER and the VAO binding; the VAO
    // went through the bind cache, the array buffer is not cached state.
    InvalidatePipelineCache();
    return ok;
}
