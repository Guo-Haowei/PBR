#version 410 core
layout (location = 0) out vec4 out_color;

in vec3 pass_position;

uniform sampler2D u_env_map;

const vec2 invAtan = vec2(0.1591, 0.3183);

vec2 sampleSphericalMap(in vec3 v)
{
    vec2 uv = vec2(atan(v.z, v.x), asin(v.y));
    uv *= invAtan;
    uv += 0.5;
    uv.y = 1.0 - uv.y;
    return uv;
}

void main()
{
    vec2 uv = sampleSphericalMap(normalize(pass_position));
    out_color = vec4(texture(u_env_map, uv).rgb, 1.0);
}
