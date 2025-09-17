#include "utils.h"
#include <glm/glm.hpp>

constexpr double RENDER_SCALE = 1e-6; // 1 render unit = 1,000,000 m

glm::vec3 toRender(glm::dvec3 physicsPos) {
    return physicsPos * RENDER_SCALE;
}

float toRender(double realDist) {
    return (float) (realDist * RENDER_SCALE);
}
