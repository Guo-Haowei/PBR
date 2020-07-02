#version 410 core
layout (location = 0) out vec4 out_color;

in vec2 pass_uv;

uniform sampler2D u_texture;

void main()
{
    vec2 color = texture(u_texture, pass_uv).rg;
    out_color = vec4(color, 0.0, 1.0);
}
