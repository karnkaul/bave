#version 450 core

struct Instance {
	mat4 transform;
	vec4 rgba;
};

layout (location = 0) in vec2 vpos;
layout (location = 1) in vec2 vuv;
layout (location = 2) in vec4 vrgba;

layout (set = 0, binding = 0) uniform View {
	mat4 view;
	mat4 projection;
};

layout (set = 0, binding = 1) readonly buffer Instances {
	Instance instances[];
};

layout (location = 0) out vec4 out_rgba;
layout (location = 1) out vec2 out_uv;

out gl_PerVertex {
	vec4 gl_Position;
};

void main() {
	const Instance instance = instances[gl_InstanceIndex];
	out_rgba = instance.rgba * vrgba;
	out_uv = vuv;
	const vec4 frag_pos = instance.transform * vec4(vpos, 0.0, 1.0);
	gl_Position = projection * view * frag_pos;
}
