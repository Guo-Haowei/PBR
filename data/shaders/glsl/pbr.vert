#version 410 core
layout (location = 0) in vec3 in_position;
layout (location = 1) in vec3 in_color;

layout (location = 0) out vec3 pass_color;

struct PerFrameBuffer
{
    mat4 view;
    mat4 projection;
};

uniform PerFrameBuffer u_per_frame;

void main()
{
    vec4 world_position = vec4(in_position, 1.0);
    gl_Position = u_per_frame.projection * u_per_frame.view * world_position;
    pass_color = in_color;
}
