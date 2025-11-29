#version 420

layout(location=0) in  vec3 a_vertex_position;
layout(location=1) in  vec3 aColor; // the color variable has attribute position 1

out vec3 ourColor; /// output a color to the fragment shader

uniform Transformations {
    mat4 PVM;
 };

void main() {
    gl_Position = PVM * vec4(a_vertex_position, 1.0);

    ourColor = aColor;
}
