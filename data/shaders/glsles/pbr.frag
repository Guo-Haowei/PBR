#version 300 es
precision highp float;
precision highp int;

layout (location = 0) out vec4 out_color;

in vec3 pass_color;

void main()
{
    out_color = vec4(pass_color, 1.0);
}
