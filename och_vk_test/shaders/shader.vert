#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(binding = 0) uniform uniform_buffer_obj{
    mat4 model;
    mat4 view;
    mat4 projection;
} ubo;

layout(location = 0) in vec3 in_position;
layout(location = 1) in vec3 in_colour;
layout(location = 2) in vec2 in_tex_position;

layout(location = 0) out vec3 frag_colour;
layout(location = 1) out vec2 frag_tex_position;

void main() {
    gl_Position = ubo.projection * ubo.view * ubo.model * vec4(in_position, 1.0);
    frag_colour = in_colour;
    frag_tex_position = in_tex_position;
}
