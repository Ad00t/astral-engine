#ifndef PHYSICS_ENGINE_H
#define PHYSICS_ENGINE_H

#include "core/update_limiter.h"
#include "core/simulation.h"
#include <glm/glm.hpp>
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/quaternion.hpp>

class PhysicsEngine {
public:
    UpdateLimiter updateLimiter;

    PhysicsEngine(double maxUpdateRate);
    ~PhysicsEngine();

    void update(Simulation& sim, double dT);
};

#endif // PHYSICS_ENGINE_H
