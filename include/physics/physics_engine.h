#ifndef PHYSICS_ENGINE_H
#define PHYSICS_ENGINE_H

#include "core/update_limiter.h"
#include "core/simulation.h"
#include <glm/glm.hpp>
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/quaternion.hpp>

class PhysicsEngine {
private:
    void updateImpl(Simulation& sim, double dt);

public:
    UpdateLimiter updateLimiter;

    struct Config {
        bool should_substep_updates = true;
        double max_update_dt = 6000.0;
        double collision_bias = 0.5;
    };
    Config config;

    PhysicsEngine(double maxUpdateRate);
    ~PhysicsEngine();

    void update(Simulation& sim, double dt);
};

#endif // PHYSICS_ENGINE_H
