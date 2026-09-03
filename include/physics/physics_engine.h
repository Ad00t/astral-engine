#ifndef PHYSICS_ENGINE_H
#define PHYSICS_ENGINE_H

#include "update_limiter.h"
#include "simulation.h"
#include <glm/glm.hpp>
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/quaternion.hpp>

class PhysicsEngine {
public:
    UpdateLimiter updateLimiter;

    PhysicsEngine(double maxUpdateRate);
    ~PhysicsEngine();

    void initialize(Simulation& sim);
    void computeForces(Simulation& sim);
    void updateRigidBodies(Simulation& sim, double dT);
};

#endif // PHYSICS_ENGINE_H
