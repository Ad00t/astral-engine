#include <glm/glm.hpp>

constexpr double G = 6.67430e-11;

glm::vec3 toRenderUnits(glm::dvec3 realPos);
float toRenderUnits(double realDist);

glm::dvec3 toRealUnits(glm::vec3 renderPos);
double toRealUnits(float renderDist);
