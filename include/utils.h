#include <glm/glm.hpp>

constexpr float RENDER_SCALE = 1e-6f; // 1 render unit = 1,000,000 m

glm::vec3 toRender(glm::vec3 physicsPos);
float toRender(float realDist);
