#version 410 core
layout (location = 0) in vec3 in_position;
layout (location = 1) in vec2 in_uv;
layout (location = 2) in vec3 in_normal;

struct VS_OUT
{
    vec3 position;
    vec3 normal;
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
    vec4 world_position = u_per_draw.transform * vec4(in_position, 1.0);
    vs_pass.position = world_position.xyz;
    vs_pass.normal = mat3(u_per_draw.transform) * in_normal;
    vs_pass.uv = in_uv;

    gl_Position = u_per_frame.projection * u_per_frame.view * world_position;
}

