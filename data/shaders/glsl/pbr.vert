#version 410 core
layout (location = 0) in vec3 in_position;
layout (location = 1) in vec2 in_uv;
layout (location = 2) in vec3 in_normal;

struct VS_OUT
{
    vec3 position;
    float metallic;
    vec3 normal;
    float roughness;
    vec2 uv;
};

out VS_OUT vs_pass;

struct PerFrameBuffer
{
    mat4 view;
    mat4 projection;
};

struct PerDrawBuffer
{
    mat4 transform;
};

uniform PerFrameBuffer u_per_frame;
uniform PerDrawBuffer u_per_draw;

void main()
{
    float spacing = 2.5;
    int count = 5;
    int ix = gl_InstanceID % count;
    int iy = gl_InstanceID / count;
    vs_pass.metallic = clamp(float(iy) / float(count), 0.05, 0.95);
    vs_pass.roughness = clamp(float(ix) / float(count), 0.05, 0.95);
    float x = spacing * (float(ix - count / 2));
    float y = spacing * (float(iy - count / 2));
    vec3 offset = vec3(x, y, 0.0);
    // vec4 world_position = u_per_draw.transform * vec4(in_position, 1.0);
    vec4 world_position = vec4(in_position + offset, 1.0);
    vs_pass.position = world_position.xyz;
    vs_pass.normal = in_normal;
    vs_pass.uv = in_uv;

    gl_Position = u_per_frame.projection * u_per_frame.view * world_position;
}
