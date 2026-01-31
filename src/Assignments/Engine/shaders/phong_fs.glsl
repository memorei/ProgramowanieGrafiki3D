#version 460

const int MAX_POINT_LIGHTS = 24;

struct PointLight {
    vec3 position_in_view_space;
    vec3 color;
    float intensity;
    float radius;
};

layout(std140, binding=0) uniform Material {
    vec4 Kd;
    bool use_map_Kd;
};

layout(std140, binding=2) uniform Lights {
    vec3 ambient;
    uint n_p_lights;
    PointLight p_light[MAX_POINT_LIGHTS];
};

layout(location=0) out vec4 vFragColor;

in vec3 vertex_normals_in_vs;
in vec3 vertex_coords_in_vs;

void main() {
    vec3 Nn = normalize(vertex_normals_in_vs);

    vec3 L = normalize(
        p_light[0].position_in_view_space - vertex_coords_in_vs
    );

    float diff = max(dot(Nn, L), 0.0);

    vec3 result = ambient + diff * p_light[0].color;

    vFragColor = vec4(result, 1.0);
}