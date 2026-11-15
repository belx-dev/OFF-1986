#version 450
// Flat shader for the Frame-on-Vulkan spike.  Matrices arrive as the
// GfxMatrix bytes the GL33 backend uploads to its UBO, so the same
// column-major interpretation (`proj * view * world * p`) applies.  GL33
// runs with glClipControl(LOWER_LEFT, ZERO_TO_ONE), so only Y flips.
layout(push_constant) uniform PC {
    mat4 world;
    mat4 viewProj;
} pc;

layout(location = 0) in vec3 inPos;
layout(location = 1) in vec3 inNormal;

layout(location = 0) out vec3 vColor;

void main() {
    vec4 clip = pc.viewProj * (pc.world * vec4(inPos, 1.0));
    clip.y = -clip.y;
    gl_Position = clip;
    vec3 n = normalize(mat3(pc.world) * inNormal);
    // Unlit "normal as colour" so structure is visible without textures.
    vColor = n * 0.5 + 0.5;
}
