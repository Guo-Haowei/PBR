#version 410 core
layout (location = 0) in vec3 in_position;

out vec3 pass_position;

struct PerFrameBuffer
{
    mat4 view;
    mat4 projection;
};

uniform PerFrameBuffer u_per_frame;

void main()
{
    pass_position = in_position;
    vec4 world_position = vec4(10.0f * in_position, 1.0);
    mat4 viewRotation = mat4(mat3(u_per_frame.view));
    vec4 clip_position = u_per_frame.projection * viewRotation * world_position;
    gl_Position = clip_position.xyww; // force depth to be 1
}


