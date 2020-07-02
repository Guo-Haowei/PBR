#version 410 core
layout (location = 0) in vec2 in_position;

out vec2 pass_uv;

void main()
{
    pass_uv = 0.5 * (in_position + 1.0);
    gl_Position = vec4(in_position, 0.0, 1.0);
}
