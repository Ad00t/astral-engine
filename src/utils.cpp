#include "utils.h"
#include <glm/glm.hpp>

const double RENDER_SCALE_FACTOR = 1e-3; // 1 render unit = 1,000 m

glm::vec3 toRenderUnits(glm::dvec3 realPos) {
    return glm::vec3(realPos * RENDER_SCALE_FACTOR);
}

float toRenderUnits(double realDist) {
    return (float) (realDist * RENDER_SCALE_FACTOR);
}

glm::dvec3 toRealUnits(glm::vec3 renderPos) {
    return glm::dvec3(renderPos) / RENDER_SCALE_FACTOR;
}

double toRealUnits(float renderDist) {
    return (double) renderDist / RENDER_SCALE_FACTOR;
}
