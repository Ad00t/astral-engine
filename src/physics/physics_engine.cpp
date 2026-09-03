#include "physics/physics_engine.h"
#include "simulation.h"
#include "glm/geometric.hpp"
#include <glm/glm.hpp>
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/quaternion.hpp>
#include <glm/gtx/string_cast.hpp>

const double G = 6.67430e-11;

PhysicsEngine::PhysicsEngine(double maxUpdateRate)
    : updateLimiter(maxUpdateRate) {}

PhysicsEngine::~PhysicsEngine() {}

void PhysicsEngine::initialize(Simulation& sim) {
    computeForces(sim);
    for (auto& [id, rb] : sim.rigidbodies) {
        rb.acc = rb.acc_new;
    }
    for (auto& [id, rb] : sim.rigidbodies) {
        rb.acc_new = glm::dvec3(0.0);
    }
}

void PhysicsEngine::computeForces(Simulation& sim) {
    for (auto it1 = sim.rigidbodies.begin(); it1 != sim.rigidbodies.end(); ++it1) {
        auto& [id1, rb1] = *it1;
        auto it2 = it1;
        ++it2;
        for (; it2 != sim.rigidbodies.end(); ++it2) {
            auto& [id2, rb2] = *it2;

            glm::dvec3 dir = rb2.pos - rb1.pos;
            double r = glm::length(dir);
            if (r < 1e-4) continue;
            glm::dvec3 F_g = (G * rb1.mass * rb2.mass / (r * r)) * glm::normalize(dir);

            rb1.applyForce(F_g);
            rb2.applyForce(-F_g);
        }
    }
}

void PhysicsEngine::updateRigidBodies(Simulation& sim, double dT) {
    for (auto& [id, rb] : sim.rigidbodies) {
        rb.integratePos(dT);
        rb.integrateRot(dT);
    }

    computeForces(sim);

    for (auto& [id, rb] : sim.rigidbodies) {
        rb.integrateVel(dT);
    }
}
