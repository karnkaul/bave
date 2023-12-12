#version 450 core

const int MAX_TEXTURES = 4;

layout (set = 1, binding = 0) uniform sampler2D in_texture;

layout (location = 0) in vec4 in_rgba;
layout (location = 1) in vec2 in_uv;

layout (location = 0) out vec4 out_rgba;

void main() {
	const vec4 rgba = texture(in_texture, in_uv);
	out_rgba = rgba * in_rgba;
}
