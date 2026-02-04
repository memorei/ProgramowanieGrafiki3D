#version 460

const int MAX_POINT_LIGHTS = 24;

struct PointLight {
    vec3 position_in_view_space;
    float intensity; 
    vec3 color;
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
    
    vec3 total_diffuse = vec3(0.0);

    for(uint i = 0; i < n_p_lights; i++) {
        vec3 L = p_light[i].position_in_view_space - vertex_coords_in_vs;
        float dist = length(L); 
        L = normalize(L);       
        
        float diff_coeff = max(dot(Nn, L), 0.0);
        
        float attenuation = 1.0 / (1.0 + (dist * dist) / (p_light[i].radius * p_light[i].radius));
        
        total_diffuse += diff_coeff * p_light[i].color * p_light[i].intensity * attenuation;
    }

    vec3 final_color = (ambient + total_diffuse) * Kd.rgb;

    vFragColor = vec4(final_color, Kd.a);
}