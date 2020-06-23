#version 410 core
layout (location = 0) in vec3 in_position;
layout (location = 1) in vec2 in_uv;
layout (location = 2) in vec3 in_normal;

out vec3 pass_normal;
out vec3 pass_position;
out vec2 pass_uv;

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
    gl_Position = u_per_frame.projection * u_per_frame.view * world_position;
    pass_position = world_position.xyz;
    pass_normal = in_normal;
    pass_uv = in_uv;
}
