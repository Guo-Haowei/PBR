#version 410 core
in vec3 pass_position;
in vec3 pass_normal;
in vec2 pass_uv;

layout (location = 0) out vec4 out_color;

const vec3 light_position = vec3(0.0, 10.0, -10.0);

void main()
{
    vec3 N = normalize(pass_normal);
    vec3 L = normalize(light_position - pass_position);
    float d = max(dot(N, L), 0.0);
    vec3 color = vec3(0.9);

    out_color = vec4(d * color, 1.0);
}
