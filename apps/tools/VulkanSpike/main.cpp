// Frame-on-Vulkan spike (off-1986-project#21).
//
// Reads a render-frame dump (FrameDumpFormat.hpp), draws its opaque world
// geometry offscreen with direct Vulkan 1.3 and a flat shader, writes a PPM
// and prints the numbers the spike note needs (code size is this file).
//
// Deliberately simple: one host-visible buffer per resource class, one
// pipeline per vertex layout, one command buffer, synchronous readback.
// Nothing here is meant to be the production backend; it measures what the
// Frame layer gives a non-GL backend and what direct Vulkan costs.

#include <Poseidon/Graphics/Rendering/Frame/FrameDumpFormat.hpp>

#include <vulkan/vulkan.h>

#include <chrono>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <map>
#include <string>
#include <tuple>
#include <vector>

namespace dump = Poseidon::render::frame::dump;

namespace
{

// ---------------------------------------------------------------- utilities

[[noreturn]] void die(const char* what, VkResult r = VK_SUCCESS)
{
    if (r != VK_SUCCESS)
        fprintf(stderr, "error: %s (VkResult %d)\n", what, static_cast<int>(r));
    else
        fprintf(stderr, "error: %s\n", what);
    exit(1);
}

void check(VkResult r, const char* what)
{
    if (r != VK_SUCCESS)
        die(what, r);
}

using Clock = std::chrono::steady_clock;
double msSince(Clock::time_point t0)
{
    return std::chrono::duration<double, std::milli>(Clock::now() - t0).count();
}

// Column-major 4x4 multiply on raw floats: the GfxMatrix bytes are read by
// GLSL as column-major, so this reproduces `a * b` as the shaders see it.
void mulColumnMajor(float out[16], const float a[16], const float b[16])
{
    for (int col = 0; col < 4; ++col)
        for (int row = 0; row < 4; ++row)
        {
            float s = 0.0f;
            for (int k = 0; k < 4; ++k)
                s += a[k * 4 + row] * b[col * 4 + k];
            out[col * 4 + row] = s;
        }
}

// ---------------------------------------------------------------- dump file

struct Mesh
{
    dump::DumpMesh desc;
    std::vector<unsigned char> vertices;
    std::vector<unsigned char> indices;
    // Filled at upload time.
    VkDeviceSize vertexOffset = 0; // bytes into the shared vertex buffer
    std::uint32_t firstIndex = 0;  // index units into the shared index buffer
    std::uint32_t indexCount = 0;
};

struct Dump
{
    dump::DumpHeader header;
    std::vector<Mesh> meshes;
    std::vector<dump::DumpDraw> draws;
};

Dump loadDump(const char* path)
{
    FILE* f = fopen(path, "rb");
    if (!f)
        die("cannot open dump");
    Dump d;
    if (fread(&d.header, sizeof(d.header), 1, f) != 1 || d.header.magic != dump::kMagic ||
        d.header.version != dump::kVersion)
        die("not a frame dump (bad magic/version)");
    d.meshes.resize(d.header.meshCount);
    for (Mesh& m : d.meshes)
    {
        if (fread(&m.desc, sizeof(m.desc), 1, f) != 1)
            die("truncated mesh header");
        m.vertices.resize(m.desc.vertexBytes);
        m.indices.resize(m.desc.indexBytes);
        if (fread(m.vertices.data(), 1, m.vertices.size(), f) != m.vertices.size() ||
            fread(m.indices.data(), 1, m.indices.size(), f) != m.indices.size())
            die("truncated mesh data");
        m.indexCount = m.desc.indexBytes / 2;
    }
    d.draws.resize(d.header.drawCount);
    if (d.header.drawCount && fread(d.draws.data(), sizeof(dump::DumpDraw), d.draws.size(), f) != d.draws.size())
        die("truncated draw list");
    fclose(f);
    return d;
}

} // namespace

#include "spike_shaders.hpp" // generated: kFlatVert/kFlatFrag SPIR-V words

// ---------------------------------------------------------------- vulkan

namespace
{

struct Options
{
    const char* dumpPath = nullptr;
    const char* outPath = nullptr;
    bool validate = false;
    int gpu = -1;
};

struct Vk
{
    VkInstance instance = VK_NULL_HANDLE;
    VkPhysicalDevice phys = VK_NULL_HANDLE;
    VkPhysicalDeviceMemoryProperties memProps{};
    VkDevice device = VK_NULL_HANDLE;
    std::uint32_t queueFamily = 0;
    VkQueue queue = VK_NULL_HANDLE;
    VkCommandPool pool = VK_NULL_HANDLE;
    float timestampPeriodNs = 0.0f;
    bool timestampsValid = false; // queue family reports timestampValidBits > 0
    std::string gpuName;
};

struct Buffer
{
    VkBuffer buffer = VK_NULL_HANDLE;
    VkDeviceMemory memory = VK_NULL_HANDLE;
    void* mapped = nullptr;
    VkDeviceSize size = 0;
};

struct Image
{
    VkImage image = VK_NULL_HANDLE;
    VkDeviceMemory memory = VK_NULL_HANDLE;
    VkImageView view = VK_NULL_HANDLE;
};

std::uint32_t findMemoryType(const Vk& vk, std::uint32_t typeBits, VkMemoryPropertyFlags want)
{
    for (std::uint32_t i = 0; i < vk.memProps.memoryTypeCount; ++i)
        if ((typeBits & (1u << i)) && (vk.memProps.memoryTypes[i].propertyFlags & want) == want)
            return i;
    die("no suitable memory type");
}

Vk createDevice(const Options& opt)
{
    Vk vk;
    VkApplicationInfo app{VK_STRUCTURE_TYPE_APPLICATION_INFO};
    app.pApplicationName = "PoseidonVulkanSpike";
    app.apiVersion = VK_API_VERSION_1_3;
    VkInstanceCreateInfo ici{VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO};
    ici.pApplicationInfo = &app;
    const char* layer = "VK_LAYER_KHRONOS_validation";
    if (opt.validate)
    {
        std::uint32_t n = 0;
        vkEnumerateInstanceLayerProperties(&n, nullptr);
        std::vector<VkLayerProperties> layers(n);
        vkEnumerateInstanceLayerProperties(&n, layers.data());
        bool have = false;
        for (const auto& l : layers)
            have = have || strcmp(l.layerName, layer) == 0;
        if (have)
        {
            ici.enabledLayerCount = 1;
            ici.ppEnabledLayerNames = &layer;
            printf("validation: VK_LAYER_KHRONOS_validation enabled\n");
        }
        else
            printf("validation: layer not installed, running without\n");
    }
    check(vkCreateInstance(&ici, nullptr, &vk.instance), "vkCreateInstance");

    std::uint32_t n = 0;
    vkEnumeratePhysicalDevices(vk.instance, &n, nullptr);
    if (n == 0)
        die("no Vulkan device");
    std::vector<VkPhysicalDevice> devs(n);
    vkEnumeratePhysicalDevices(vk.instance, &n, devs.data());
    // Prefer a discrete GPU unless --gpu picks one.
    int pick = opt.gpu;
    if (pick < 0)
    {
        pick = 0;
        for (std::uint32_t i = 0; i < n; ++i)
        {
            VkPhysicalDeviceProperties p;
            vkGetPhysicalDeviceProperties(devs[i], &p);
            if (p.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU)
            {
                pick = static_cast<int>(i);
                break;
            }
        }
    }
    if (pick >= static_cast<int>(n))
        die("--gpu index out of range");
    vk.phys = devs[static_cast<size_t>(pick)];
    VkPhysicalDeviceProperties props;
    vkGetPhysicalDeviceProperties(vk.phys, &props);
    vk.gpuName = props.deviceName;
    vk.timestampPeriodNs = props.limits.timestampPeriod;
    if (props.apiVersion < VK_API_VERSION_1_3)
        die("device does not support Vulkan 1.3");
    vkGetPhysicalDeviceMemoryProperties(vk.phys, &vk.memProps);

    std::uint32_t qn = 0;
    vkGetPhysicalDeviceQueueFamilyProperties(vk.phys, &qn, nullptr);
    std::vector<VkQueueFamilyProperties> qf(qn);
    vkGetPhysicalDeviceQueueFamilyProperties(vk.phys, &qn, qf.data());
    bool found = false;
    for (std::uint32_t i = 0; i < qn && !found; ++i)
        if (qf[i].queueFlags & VK_QUEUE_GRAPHICS_BIT)
        {
            vk.queueFamily = i;
            vk.timestampsValid = qf[i].timestampValidBits > 0;
            found = true;
        }
    if (!found)
        die("no graphics queue");

    const float prio = 1.0f;
    VkDeviceQueueCreateInfo qci{VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO};
    qci.queueFamilyIndex = vk.queueFamily;
    qci.queueCount = 1;
    qci.pQueuePriorities = &prio;
    VkPhysicalDeviceVulkan13Features f13{VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_3_FEATURES};
    f13.dynamicRendering = VK_TRUE;
    f13.synchronization2 = VK_TRUE;
    VkDeviceCreateInfo dci{VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO};
    dci.pNext = &f13;
    dci.queueCreateInfoCount = 1;
    dci.pQueueCreateInfos = &qci;
    check(vkCreateDevice(vk.phys, &dci, nullptr, &vk.device), "vkCreateDevice");
    vkGetDeviceQueue(vk.device, vk.queueFamily, 0, &vk.queue);

    VkCommandPoolCreateInfo pci{VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO};
    pci.queueFamilyIndex = vk.queueFamily;
    check(vkCreateCommandPool(vk.device, &pci, nullptr, &vk.pool), "vkCreateCommandPool");
    return vk;
}

Buffer createBuffer(const Vk& vk, VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags mem)
{
    Buffer b;
    b.size = size;
    VkBufferCreateInfo bci{VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO};
    bci.size = size;
    bci.usage = usage;
    check(vkCreateBuffer(vk.device, &bci, nullptr, &b.buffer), "vkCreateBuffer");
    VkMemoryRequirements req;
    vkGetBufferMemoryRequirements(vk.device, b.buffer, &req);
    VkMemoryAllocateInfo mai{VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO};
    mai.allocationSize = req.size;
    mai.memoryTypeIndex = findMemoryType(vk, req.memoryTypeBits, mem);
    check(vkAllocateMemory(vk.device, &mai, nullptr, &b.memory), "vkAllocateMemory(buffer)");
    check(vkBindBufferMemory(vk.device, b.buffer, b.memory, 0), "vkBindBufferMemory");
    if (mem & VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT)
        check(vkMapMemory(vk.device, b.memory, 0, size, 0, &b.mapped), "vkMapMemory");
    return b;
}

Image createImage(const Vk& vk, std::uint32_t w, std::uint32_t h, VkFormat format, VkImageUsageFlags usage,
                  VkImageAspectFlags aspect)
{
    Image img;
    VkImageCreateInfo ici{VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO};
    ici.imageType = VK_IMAGE_TYPE_2D;
    ici.format = format;
    ici.extent = {w, h, 1};
    ici.mipLevels = 1;
    ici.arrayLayers = 1;
    ici.samples = VK_SAMPLE_COUNT_1_BIT;
    ici.tiling = VK_IMAGE_TILING_OPTIMAL;
    ici.usage = usage;
    ici.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    check(vkCreateImage(vk.device, &ici, nullptr, &img.image), "vkCreateImage");
    VkMemoryRequirements req;
    vkGetImageMemoryRequirements(vk.device, img.image, &req);
    VkMemoryAllocateInfo mai{VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO};
    mai.allocationSize = req.size;
    mai.memoryTypeIndex = findMemoryType(vk, req.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
    check(vkAllocateMemory(vk.device, &mai, nullptr, &img.memory), "vkAllocateMemory(image)");
    check(vkBindImageMemory(vk.device, img.image, img.memory, 0), "vkBindImageMemory");
    VkImageViewCreateInfo vci{VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO};
    vci.image = img.image;
    vci.viewType = VK_IMAGE_VIEW_TYPE_2D;
    vci.format = format;
    vci.subresourceRange = {aspect, 0, 1, 0, 1};
    check(vkCreateImageView(vk.device, &vci, nullptr, &img.view), "vkCreateImageView");
    return img;
}

VkShaderModule createShader(const Vk& vk, const std::uint32_t* words, size_t count)
{
    VkShaderModuleCreateInfo ci{VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO};
    ci.codeSize = count * 4;
    ci.pCode = words;
    VkShaderModule m;
    check(vkCreateShaderModule(vk.device, &ci, nullptr, &m), "vkCreateShaderModule");
    return m;
}

// Pipeline key: vertex layout (stride + position/normal offsets) plus the
// two descriptor fields the flat shader still honours, blend and depth mode.
struct Layout
{
    std::uint32_t stride = 0;
    std::uint32_t posOffset = 0;
    std::uint32_t normalOffset = 0; // = posOffset when the VAO had no normal
    std::uint32_t blend = 0;        // BlendMode: 0 Opaque, 1 AlphaBlend, 2 Additive
    std::uint32_t depth = 0;        // DepthMode: 0 Normal, 1 ReadOnly, 2 Disabled
    bool operator<(const Layout& o) const
    {
        return std::tie(stride, posOffset, normalOffset, blend, depth) <
               std::tie(o.stride, o.posOffset, o.normalOffset, o.blend, o.depth);
    }
};

Layout layoutOf(const dump::DumpMesh& m, const dump::DumpDraw& d)
{
    Layout l;
    l.blend = d.blend;
    l.depth = d.depth;
    l.stride = m.stride;
    if (!m.attribs[0].enabled || m.attribs[0].components != 3 || m.attribs[0].type != 0x1406 /*GL_FLOAT*/)
        die("mesh position attribute is not 3 x GL_FLOAT; the spike only handles that layout");
    l.posOffset = m.attribs[0].offset;
    const auto& n = m.attribs[1];
    const bool hasNormal = n.enabled && n.components == 3 && n.type == 0x1406 /*GL_FLOAT*/;
    l.normalOffset = hasNormal ? n.offset : l.posOffset;
    return l;
}

VkPipeline createPipeline(const Vk& vk, VkPipelineLayout layout, VkShaderModule vs, VkShaderModule fs,
                          VkFormat colorFormat, VkFormat depthFormat, const Layout& vl)
{
    VkPipelineShaderStageCreateInfo stages[2]{};
    stages[0].sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    stages[0].stage = VK_SHADER_STAGE_VERTEX_BIT;
    stages[0].module = vs;
    stages[0].pName = "main";
    stages[1] = stages[0];
    stages[1].stage = VK_SHADER_STAGE_FRAGMENT_BIT;
    stages[1].module = fs;

    VkVertexInputBindingDescription binding{0, vl.stride, VK_VERTEX_INPUT_RATE_VERTEX};
    VkVertexInputAttributeDescription attrs[2] = {
        {0, 0, VK_FORMAT_R32G32B32_SFLOAT, vl.posOffset},
        {1, 0, VK_FORMAT_R32G32B32_SFLOAT, vl.normalOffset},
    };
    VkPipelineVertexInputStateCreateInfo vi{VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO};
    vi.vertexBindingDescriptionCount = 1;
    vi.pVertexBindingDescriptions = &binding;
    vi.vertexAttributeDescriptionCount = 2;
    vi.pVertexAttributeDescriptions = attrs;

    VkPipelineInputAssemblyStateCreateInfo ia{VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO};
    ia.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
    VkPipelineViewportStateCreateInfo vp{VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO};
    vp.viewportCount = 1;
    vp.scissorCount = 1;
    VkPipelineRasterizationStateCreateInfo rs{VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO};
    rs.polygonMode = VK_POLYGON_MODE_FILL;
    rs.cullMode = VK_CULL_MODE_NONE; // spike: ignore per-draw cull (see note)
    rs.frontFace = VK_FRONT_FACE_CLOCKWISE;
    rs.lineWidth = 1.0f;
    VkPipelineMultisampleStateCreateInfo ms{VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO};
    ms.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
    VkPipelineDepthStencilStateCreateInfo ds{VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO};
    ds.depthTestEnable = VK_TRUE;
    ds.depthWriteEnable = vl.depth == 0 ? VK_TRUE : VK_FALSE;
    ds.depthCompareOp = vl.depth == 2 ? VK_COMPARE_OP_ALWAYS : VK_COMPARE_OP_LESS_OR_EQUAL;
    VkPipelineColorBlendAttachmentState blend{};
    blend.colorWriteMask = 0xF;
    if (vl.blend == 1 || vl.blend == 2)
    {
        blend.blendEnable = VK_TRUE;
        blend.srcColorBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA;
        blend.dstColorBlendFactor = vl.blend == 1 ? VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA : VK_BLEND_FACTOR_ONE;
        blend.colorBlendOp = VK_BLEND_OP_ADD;
        blend.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
        blend.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
        blend.alphaBlendOp = VK_BLEND_OP_ADD;
    }
    VkPipelineColorBlendStateCreateInfo cb{VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO};
    cb.attachmentCount = 1;
    cb.pAttachments = &blend;
    const VkDynamicState dyn[] = {VK_DYNAMIC_STATE_VIEWPORT, VK_DYNAMIC_STATE_SCISSOR};
    VkPipelineDynamicStateCreateInfo dsci{VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO};
    dsci.dynamicStateCount = 2;
    dsci.pDynamicStates = dyn;

    VkPipelineRenderingCreateInfo rendering{VK_STRUCTURE_TYPE_PIPELINE_RENDERING_CREATE_INFO};
    rendering.colorAttachmentCount = 1;
    rendering.pColorAttachmentFormats = &colorFormat;
    rendering.depthAttachmentFormat = depthFormat;

    VkGraphicsPipelineCreateInfo gp{VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO};
    gp.pNext = &rendering;
    gp.stageCount = 2;
    gp.pStages = stages;
    gp.pVertexInputState = &vi;
    gp.pInputAssemblyState = &ia;
    gp.pViewportState = &vp;
    gp.pRasterizationState = &rs;
    gp.pMultisampleState = &ms;
    gp.pDepthStencilState = &ds;
    gp.pColorBlendState = &cb;
    gp.pDynamicState = &dsci;
    gp.layout = layout;
    VkPipeline p;
    check(vkCreateGraphicsPipelines(vk.device, VK_NULL_HANDLE, 1, &gp, nullptr, &p), "vkCreateGraphicsPipelines");
    return p;
}

// FramePassKind values drawn by the spike, in Frame order: WorldOpaque(3),
// WorldCutout(4), SurfaceOverlay(5), WorldTransparent(7).  Sky, water,
// shadows, cockpit and HUD are skipped.  Shadow-blend draws (BlendMode 3)
// are skipped too: they need the stencil path.
bool isWorldPass(std::uint32_t passKind)
{
    return passKind == 3 || passKind == 4 || passKind == 5 || passKind == 7;
}

struct PushConstants
{
    float world[16];
    float viewProj[16];
};

void writePPM(const char* path, const unsigned char* rgba, std::uint32_t w, std::uint32_t h, std::uint32_t rowPitch)
{
    FILE* f = fopen(path, "wb");
    if (!f)
        die("cannot write output");
    fprintf(f, "P6\n%u %u\n255\n", w, h);
    std::vector<unsigned char> row(w * 3);
    for (std::uint32_t y = 0; y < h; ++y)
    {
        const unsigned char* src = rgba + y * rowPitch;
        for (std::uint32_t x = 0; x < w; ++x)
            memcpy(&row[x * 3], src + x * 4, 3);
        fwrite(row.data(), 1, row.size(), f);
    }
    fclose(f);
}

Options parseArgs(int argc, char** argv)
{
    Options o;
    for (int i = 1; i < argc; ++i)
    {
        std::string a = argv[i];
        if (a == "--validate")
            o.validate = true;
        else if (a == "--gpu" && i + 1 < argc)
            o.gpu = atoi(argv[++i]);
        else if (!o.dumpPath)
            o.dumpPath = argv[i];
        else if (!o.outPath)
            o.outPath = argv[i];
        else
            die("usage: PoseidonVulkanSpike <dump> <out.ppm> [--validate] [--gpu N]");
    }
    if (!o.dumpPath || !o.outPath)
        die("usage: PoseidonVulkanSpike <dump> <out.ppm> [--validate] [--gpu N]");
    return o;
}

} // namespace

int main(int argc, char** argv)
{
    const Options opt = parseArgs(argc, argv);
    const auto tStart = Clock::now();

    Dump d = loadDump(opt.dumpPath);
    const std::uint32_t width = static_cast<std::uint32_t>(d.header.viewport[2]);
    const std::uint32_t height = static_cast<std::uint32_t>(d.header.viewport[3]);
    if (width == 0 || height == 0)
        die("dump has an empty viewport");
    const double tLoad = msSince(tStart);

    Vk vk = createDevice(opt);
    const double tDevice = msSince(tStart);

    // Upload: pack every mesh into one vertex and one index buffer.
    VkDeviceSize vbTotal = 0, ibTotal = 0;
    for (Mesh& m : d.meshes)
    {
        m.vertexOffset = vbTotal;
        m.firstIndex = static_cast<std::uint32_t>(ibTotal / 2);
        vbTotal += (m.vertices.size() + 15) & ~VkDeviceSize(15);
        ibTotal += (m.indices.size() + 3) & ~VkDeviceSize(3);
    }
    if (vbTotal == 0 || ibTotal == 0)
        die("dump contains no mesh data");
    const VkMemoryPropertyFlags hostMem = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;
    Buffer vb = createBuffer(vk, vbTotal, VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, hostMem);
    Buffer ib = createBuffer(vk, ibTotal, VK_BUFFER_USAGE_INDEX_BUFFER_BIT, hostMem);
    for (const Mesh& m : d.meshes)
    {
        memcpy(static_cast<unsigned char*>(vb.mapped) + m.vertexOffset, m.vertices.data(), m.vertices.size());
        memcpy(static_cast<unsigned char*>(ib.mapped) + VkDeviceSize(m.firstIndex) * 2, m.indices.data(),
               m.indices.size());
    }
    const double tUpload = msSince(tStart);

    // Targets.
    const VkFormat colorFormat = VK_FORMAT_R8G8B8A8_UNORM;
    const VkFormat depthFormat = VK_FORMAT_D32_SFLOAT;
    Image color =
        createImage(vk, width, height, colorFormat,
                    VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT, VK_IMAGE_ASPECT_COLOR_BIT);
    Image depth = createImage(vk, width, height, depthFormat, VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT,
                              VK_IMAGE_ASPECT_DEPTH_BIT);
    Buffer readback = createBuffer(vk, VkDeviceSize(width) * height * 4, VK_BUFFER_USAGE_TRANSFER_DST_BIT, hostMem);

    // Pipelines: one per distinct vertex layout.
    VkShaderModule vs = createShader(vk, kFlatVert, kFlatVert_words);
    VkShaderModule fs = createShader(vk, kFlatFrag, kFlatFrag_words);
    VkPushConstantRange pcr{VK_SHADER_STAGE_VERTEX_BIT, 0, sizeof(PushConstants)};
    VkPipelineLayoutCreateInfo plci{VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO};
    plci.pushConstantRangeCount = 1;
    plci.pPushConstantRanges = &pcr;
    VkPipelineLayout pipelineLayout;
    check(vkCreatePipelineLayout(vk.device, &plci, nullptr, &pipelineLayout), "vkCreatePipelineLayout");
    std::map<Layout, VkPipeline> pipelines;
    std::map<std::uint32_t, const Mesh*> meshByVao;
    for (const Mesh& m : d.meshes)
        meshByVao[m.desc.vao] = &m;
    for (const dump::DumpDraw& dr : d.draws)
    {
        auto it = meshByVao.find(dr.vao);
        if (!isWorldPass(dr.passKind) || dr.blend == 3 || it == meshByVao.end())
            continue;
        const Layout l = layoutOf(it->second->desc, dr);
        if (!pipelines.count(l))
            pipelines[l] = createPipeline(vk, pipelineLayout, vs, fs, colorFormat, depthFormat, l);
    }
    const double tPipelines = msSince(tStart);

    // Timestamps for GPU time.
    VkQueryPoolCreateInfo qpci{VK_STRUCTURE_TYPE_QUERY_POOL_CREATE_INFO};
    qpci.queryType = VK_QUERY_TYPE_TIMESTAMP;
    qpci.queryCount = 2;
    VkQueryPool queries;
    check(vkCreateQueryPool(vk.device, &qpci, nullptr, &queries), "vkCreateQueryPool");

    // Record.
    VkCommandBufferAllocateInfo cbai{VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO};
    cbai.commandPool = vk.pool;
    cbai.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    cbai.commandBufferCount = 1;
    VkCommandBuffer cmd;
    check(vkAllocateCommandBuffers(vk.device, &cbai, &cmd), "vkAllocateCommandBuffers");
    VkCommandBufferBeginInfo bi{VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO};
    bi.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
    check(vkBeginCommandBuffer(cmd, &bi), "vkBeginCommandBuffer");
    vkCmdResetQueryPool(cmd, queries, 0, 2);
    vkCmdWriteTimestamp(cmd, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, queries, 0);

    // UNDEFINED -> attachment layouts.
    VkImageMemoryBarrier2 toAttach[2]{};
    toAttach[0].sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER_2;
    toAttach[0].dstStageMask = VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT;
    toAttach[0].dstAccessMask = VK_ACCESS_2_COLOR_ATTACHMENT_WRITE_BIT;
    toAttach[0].newLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
    toAttach[0].image = color.image;
    toAttach[0].subresourceRange = {VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1};
    toAttach[1] = toAttach[0];
    toAttach[1].dstStageMask = VK_PIPELINE_STAGE_2_EARLY_FRAGMENT_TESTS_BIT;
    toAttach[1].dstAccessMask = VK_ACCESS_2_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
    toAttach[1].newLayout = VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL;
    toAttach[1].image = depth.image;
    toAttach[1].subresourceRange = {VK_IMAGE_ASPECT_DEPTH_BIT, 0, 1, 0, 1};
    VkDependencyInfo dep{VK_STRUCTURE_TYPE_DEPENDENCY_INFO};
    dep.imageMemoryBarrierCount = 2;
    dep.pImageMemoryBarriers = toAttach;
    vkCmdPipelineBarrier2(cmd, &dep);

    VkRenderingAttachmentInfo colorAtt{VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO};
    colorAtt.imageView = color.view;
    colorAtt.imageLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
    colorAtt.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    colorAtt.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
    // Exactly representable in UNORM8 so every driver clears to (26,31,41).
    colorAtt.clearValue.color = {{26.0f / 255.0f, 31.0f / 255.0f, 41.0f / 255.0f, 1.0f}};
    VkRenderingAttachmentInfo depthAtt{VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO};
    depthAtt.imageView = depth.view;
    depthAtt.imageLayout = VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL;
    depthAtt.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    depthAtt.storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    depthAtt.clearValue.depthStencil = {1.0f, 0};
    VkRenderingInfo ri{VK_STRUCTURE_TYPE_RENDERING_INFO};
    ri.renderArea = {{0, 0}, {width, height}};
    ri.layerCount = 1;
    ri.colorAttachmentCount = 1;
    ri.pColorAttachments = &colorAtt;
    ri.pDepthAttachment = &depthAtt;
    vkCmdBeginRendering(cmd, &ri);

    const VkViewport viewport{0.0f, 0.0f, float(width), float(height), 0.0f, 1.0f};
    const VkRect2D scissor{{0, 0}, {width, height}};
    vkCmdSetViewport(cmd, 0, 1, &viewport);
    vkCmdSetScissor(cmd, 0, 1, &scissor);
    vkCmdBindIndexBuffer(cmd, ib.buffer, 0, VK_INDEX_TYPE_UINT16);

    PushConstants pc{};
    mulColumnMajor(pc.viewProj, d.header.projection, d.header.view);

    std::uint32_t drawsIssued = 0, drawsSkipped = 0, triangles = 0;
    VkPipeline bound = VK_NULL_HANDLE;
    const Mesh* boundMesh = nullptr;
    for (const dump::DumpDraw& dr : d.draws)
    {
        if (!isWorldPass(dr.passKind) || dr.blend == 3)
        {
            ++drawsSkipped;
            continue;
        }
        auto it = meshByVao.find(dr.vao);
        if (it == meshByVao.end() || dr.indexBegin + dr.indexCount > it->second->indexCount)
        {
            ++drawsSkipped;
            continue;
        }
        const Mesh* m = it->second;
        VkPipeline p = pipelines[layoutOf(m->desc, dr)];
        if (p != bound)
        {
            vkCmdBindPipeline(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, p);
            bound = p;
        }
        if (m != boundMesh)
        {
            vkCmdBindVertexBuffers(cmd, 0, 1, &vb.buffer, &m->vertexOffset);
            boundMesh = m;
        }
        memcpy(pc.world, dr.world, sizeof(pc.world));
        vkCmdPushConstants(cmd, pipelineLayout, VK_SHADER_STAGE_VERTEX_BIT, 0, sizeof(pc), &pc);
        vkCmdDrawIndexed(cmd, dr.indexCount, 1, m->firstIndex + dr.indexBegin, 0, 0);
        ++drawsIssued;
        triangles += dr.indexCount / 3;
    }
    vkCmdEndRendering(cmd);

    // Colour -> transfer source, copy to the readback buffer.
    VkImageMemoryBarrier2 toCopy{VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER_2};
    toCopy.srcStageMask = VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT;
    toCopy.srcAccessMask = VK_ACCESS_2_COLOR_ATTACHMENT_WRITE_BIT;
    toCopy.dstStageMask = VK_PIPELINE_STAGE_2_COPY_BIT;
    toCopy.dstAccessMask = VK_ACCESS_2_TRANSFER_READ_BIT;
    toCopy.oldLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
    toCopy.newLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
    toCopy.image = color.image;
    toCopy.subresourceRange = {VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1};
    dep.imageMemoryBarrierCount = 1;
    dep.pImageMemoryBarriers = &toCopy;
    vkCmdPipelineBarrier2(cmd, &dep);
    VkBufferImageCopy region{};
    region.imageSubresource = {VK_IMAGE_ASPECT_COLOR_BIT, 0, 0, 1};
    region.imageExtent = {width, height, 1};
    vkCmdCopyImageToBuffer(cmd, color.image, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, readback.buffer, 1, &region);
    vkCmdWriteTimestamp(cmd, VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT, queries, 1);
    check(vkEndCommandBuffer(cmd), "vkEndCommandBuffer");
    const double tRecord = msSince(tStart);

    VkFenceCreateInfo fci{VK_STRUCTURE_TYPE_FENCE_CREATE_INFO};
    VkFence fence;
    check(vkCreateFence(vk.device, &fci, nullptr, &fence), "vkCreateFence");
    VkSubmitInfo si{VK_STRUCTURE_TYPE_SUBMIT_INFO};
    si.commandBufferCount = 1;
    si.pCommandBuffers = &cmd;
    check(vkQueueSubmit(vk.queue, 1, &si, fence), "vkQueueSubmit");
    check(vkWaitForFences(vk.device, 1, &fence, VK_TRUE, UINT64_MAX), "vkWaitForFences");
    const double tGpuDone = msSince(tStart);

    std::uint64_t ts[2] = {0, 0};
    check(vkGetQueryPoolResults(vk.device, queries, 0, 2, sizeof(ts), ts, sizeof(std::uint64_t),
                                VK_QUERY_RESULT_64_BIT | VK_QUERY_RESULT_WAIT_BIT),
          "vkGetQueryPoolResults");
    const double gpuMs = vk.timestampsValid ? double(ts[1] - ts[0]) * vk.timestampPeriodNs * 1e-6 : -1.0;

    // Coverage: pixels that differ from the clear colour.
    const auto* px = static_cast<const unsigned char*>(readback.mapped);
    std::uint64_t covered = 0;
    for (std::uint64_t i = 0; i < std::uint64_t(width) * height; ++i)
    {
        const unsigned char* p = px + i * 4;
        if (!(p[0] == 26 && p[1] == 31 && p[2] == 41))
            ++covered;
    }
    const double coverage = 100.0 * double(covered) / (double(width) * height);
    writePPM(opt.outPath, px, width, height, width * 4);

    printf("gpu: %s\n", vk.gpuName.c_str());
    printf("dump: %u meshes, %u draws, viewport %ux%u, vertex bytes %llu, index bytes %llu\n", d.header.meshCount,
           d.header.drawCount, width, height, static_cast<unsigned long long>(vbTotal),
           static_cast<unsigned long long>(ibTotal));
    printf("draws issued: %u (skipped %u: sky/water/shadow/HUD pass or unresolved mesh), triangles: %u, pipelines: "
           "%zu\n",
           drawsIssued, drawsSkipped, triangles, pipelines.size());
    printf("time ms: load %.1f | device %.1f | upload %.1f | pipelines %.1f | record %.1f | gpu done %.1f\n", tLoad,
           tDevice, tUpload, tPipelines, tRecord, tGpuDone);
    printf("gpu time ms: %.3f%s\n", gpuMs, vk.timestampsValid ? "" : " (timestamps unsupported)");
    printf("coverage: %.2f%% of pixels touched by geometry -> %s\n", coverage, opt.outPath);

    vkDeviceWaitIdle(vk.device);
    // Teardown left to process exit on purpose (spike).  Validation, when
    // enabled, would report the leaked objects; that is the one message
    // this program accepts.
    return coverage >= 1.0 ? 0 : 2;
}
