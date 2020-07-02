#version 410 core
layout (location = 0) out vec4 out_color;

in vec3 pass_position;

uniform samplerCube u_env_map;

void main()
{
    vec3 uvw = pass_position;
    vec3 env_color = textureLod(u_env_map, uvw, 0.0).rgb;
    env_color = env_color / (env_color + vec3(1.0));
    env_color = pow(env_color, vec3(1.0 / 2.2));

    out_color = vec4(env_color, 1.0);
}
