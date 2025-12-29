#version 420

layout(std140, binding = 1) uniform Transformations {
    mat4 PVM;
};

layout(location=0) in vec3 a_vertex_position;

void main() {
    gl_Position = PVM * vec4(a_vertex_position, 1.0);
}
