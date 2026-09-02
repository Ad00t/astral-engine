#ifndef PHYSICS_ENGINE_H
#define PHYSICS_ENGINE_H

#include "physics/rigidbody.h"
#include "update_limiter.h"
#include <glm/glm.hpp>
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/quaternion.hpp>
#include <unordered_map>
#include <string>

class PhysicsEngine {
public:
    UpdateLimiter updateLimiter;

    PhysicsEngine(double maxUpdateRate);
    ~PhysicsEngine();

    void initialize(std::unordered_map<std::string, RigidBody>& rigidbodies);
    void computeForces(std::unordered_map<std::string, RigidBody>& rigidbodies);
    void updateRigidBodies(std::unordered_map<std::string, RigidBody>& rigidbodies, double dT);
};

#endif // PHYSICS_ENGINE_H
