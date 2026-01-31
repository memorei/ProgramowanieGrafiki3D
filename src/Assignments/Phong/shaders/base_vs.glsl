#version 420

layout(std140, binding = 1) uniform Transformations {
    mat4 PVM;
};

layout(location=0) in vec3 a_vertex_position;
layout(location=1) in vec2 a_texcoord;

out vec2 vertex_texcoords;

void main() {
    gl_Position = PVM * vec4(a_vertex_position, 1.0);
    vertex_texcoords = a_texcoord;
}
