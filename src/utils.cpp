#include "utils.h"
#include <glm/glm.hpp>

constexpr double RENDER_SCALE = 1e-3; // 1 render unit = 1,000 m

glm::vec3 toRenderUnits(glm::dvec3 realPos) {
    return glm::vec3(realPos * RENDER_SCALE);
}

float toRenderUnits(double realDist) {
    return (float) (realDist * RENDER_SCALE);
}

glm::dvec3 toRealUnits(glm::vec3 renderPos) {
    return glm::dvec3(renderPos) / RENDER_SCALE;
}

double toRealUnits(float renderDist) {
    return (double) renderDist / RENDER_SCALE;
}
