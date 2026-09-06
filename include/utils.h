#include <glm/glm.hpp>

glm::vec3 toRenderUnits(glm::dvec3 realPos);
float toRenderUnits(double realDist);

glm::dvec3 toRealUnits(glm::vec3 renderPos);
double toRealUnits(float renderDist);
