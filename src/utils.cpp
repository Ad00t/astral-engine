#include "utils.h"
#include <glm/glm.hpp>

glm::vec3 toRender(glm::vec3 physicsPos) {
    return physicsPos * RENDER_SCALE;
}

float toRender(float realDist) {
    return realDist * RENDER_SCALE;
}
