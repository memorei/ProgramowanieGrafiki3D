#pragma once
#include <glm/glm.hpp>

namespace xe {
    struct PointLight {
        alignas(16) glm::vec3 position_in_vs;
        float intensity; // Wype³nia 4 bajty po vec3 (³¹cznie 16)

        alignas(16) glm::vec3 color;
        float radius;    // Wype³nia 4 bajty po vec3 (³¹cznie 32)

        PointLight() = default;
        PointLight(const glm::vec3& pos_vs, const glm::vec3& c, float i, float r)
            : position_in_vs(pos_vs), color(c), intensity(i), radius(r) {
        }
    };
}