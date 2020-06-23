#pragma once
namespace generated {

static const char* pbr_vert_c_str =
	"#version 300 es\n"\
	"precision highp float;\n"\
	"precision highp int;\n"\
	"layout (location = 0) in vec3 in_position;\n"\
	"layout (location = 1) in vec2 in_uv;\n"\
	"layout (location = 2) in vec3 in_normal;\n"\
	"out vec3 pass_normal;\n"\
	"out vec3 pass_position;\n"\
	"out vec2 pass_uv;\n"\
	"struct PerFrameBuffer\n"\
	"{\n"\
	"    mat4 view;\n"\
	"    mat4 projection;\n"\
	"};\n"\
	"uniform PerFrameBuffer u_per_frame;\n"\
	"void main()\n"\
	"{\n"\
	"    vec4 world_position = vec4(in_position, 1.0);\n"\
	"    gl_Position = u_per_frame.projection * u_per_frame.view * world_position;\n"\
	"    pass_position = world_position.xyz;\n"\
	"    pass_normal = in_normal;\n"\
	"    pass_uv = in_uv;\n"\
	"}\n"\
	"\n";

static const char* pbr_frag_c_str =
	"#version 300 es\n"\
	"precision highp float;\n"\
	"precision highp int;\n"\
	"in vec3 pass_position;\n"\
	"in vec3 pass_normal;\n"\
	"in vec2 pass_uv;\n"\
	"layout (location = 0) out vec4 out_color;\n"\
	"const vec3 light_position = vec3(0.0, 10.0, 10.0);\n"\
	"void main()\n"\
	"{\n"\
	"    vec3 N = normalize(pass_normal);\n"\
	"    vec3 L = normalize(light_position - pass_position);\n"\
	"    float d = max(dot(N, L), 0.0);\n"\
	"    vec3 color = vec3(0.9);\n"\
	"    out_color = vec4(d * color, 1.0);\n"\
	"}\n"\
	"\n";

} // namespace generated
