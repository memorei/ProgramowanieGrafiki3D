#version 420

layout(location=0) out vec4 vFragColor;

layout(std140, binding = 0) uniform Modifiers {
    float strength;
    vec3 color;
};

void main() {
    vFragColor = vec4(color * strength, 1.0);
}
