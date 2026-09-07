#ifndef PHYSICS_ENGINE_H
#define PHYSICS_ENGINE_H

#include "core/update_limiter.h"
#include "core/simulation.h"
#include <glm/glm.hpp>
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/quaternion.hpp>

static constexpr double MAX_UPDATE_DT = 6000.0;
static constexpr double G = 6.67430e-11;
static constexpr double COLLISION_BIAS = 0.5;

class PhysicsEngine {
public:
    UpdateLimiter updateLimiter;

    PhysicsEngine(double maxUpdateRate);
    ~PhysicsEngine();

    void update(Simulation& sim, double dT);
};

#endif // PHYSICS_ENGINE_H
