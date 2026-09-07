#include "physics/physics_engine.h"
#include "core/simulation.h"
#include "glm/geometric.hpp"
#include <glm/glm.hpp>
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/quaternion.hpp>
#include <glm/gtx/norm.hpp>
#include <glm/gtx/string_cast.hpp>

const double G = 6.67430e-11;
const double COLLISION_BIAS = 0.5;

PhysicsEngine::PhysicsEngine(double maxUpdateRate)
    : updateLimiter(maxUpdateRate) {}

PhysicsEngine::~PhysicsEngine() {}

void PhysicsEngine::update(Simulation& sim, double dT) {
    // Compute forces
    for (auto it1 = sim.rigidbodies.begin(); it1 != sim.rigidbodies.end(); ++it1) {
        auto& [id1, rb1] = *it1;
        auto it2 = it1; ++it2;
        for (; it2 != sim.rigidbodies.end(); ++it2) {
            auto& [id2, rb2] = *it2;

            glm::dvec3 dir = rb2.pos - rb1.pos;
            double r = glm::length(dir);
            if (r < 1e-4) continue;
            glm::dvec3 F_g = (G * rb1.mass * rb2.mass / (r * r)) * glm::normalize(dir);

            rb1.acc_new += F_g / rb1.mass;
            rb2.acc_new += F_g / rb2.mass;
        }
    }
   
    // Integrate velocities
    for (auto& [id, rb] : sim.rigidbodies) {
        rb.vel = rb.vel + 0.5 * (rb.acc + rb.acc_new) * dT;
        rb.acc = rb.acc_new;
        rb.acc_new = glm::dvec3(0.0);
    }
    
    // Check collisions
    for (auto it1 = sim.colliders.begin(); it1 != sim.colliders.end(); ++it1) {
        auto& [id1, coll1] = *it1;
        if (!sim.rigidbodies.contains(id1)) continue;
        RigidBody& rb1 = sim.rigidbodies.at(id1);
        auto it2 = it1; ++it2;
        for (; it2 != sim.colliders.end(); ++it2) {
            auto& [id2, coll2] = *it2;
            if (!sim.rigidbodies.contains(id2)) continue;
            RigidBody& rb2 = sim.rigidbodies.at(id2);

            glm::dvec3 mtv = coll1->computeMTV(coll2.get());
            if (glm::length2(mtv) <= 1e-12) continue;
        
            glm::dvec3 relative_speed = rb2.vel - rb1.vel;
            glm::dvec3 collision_normal = glm::normalize(mtv);
            double constraint_speed = glm::dot(collision_normal, relative_speed);
            if (constraint_speed > 0) { 
                double reduced_mass = 1.0 / (1.0/rb1.mass + 1.0/rb2.mass);
                double elasticity = coll1->restitution * coll2->restitution;
                double j = (constraint_speed * (1.0 + elasticity)
                            + COLLISION_BIAS / dT * glm::length(mtv)) * reduced_mass;
                glm::dvec3 impulse = j * collision_normal;
                rb1.vel += impulse / rb1.mass;
                rb2.vel -= impulse / rb2.mass;
            }
        }
    }

    // Integrate positions & rotations
    for (auto& [id, rb] : sim.rigidbodies) {
        rb.pos = rb.pos + rb.vel * dT + 0.5 * rb.acc * dT*dT;

        double ang_vel_norm = glm::l2Norm(rb.ang_vel);
        double theta = ang_vel_norm * dT;
        glm::dvec3 u_hat = rb.ang_vel / ang_vel_norm;
        glm::quat dq(1.0, 0.0, 0.0, 0.0);
        if (theta > 0) {
            dq = glm::quat(
                glm::cos(theta/2), 
                u_hat.x * glm::sin(theta/2),
                u_hat.y * glm::sin(theta/2),
                u_hat.z * glm::sin(theta/2)
            );
        }
        rb.rot = glm::normalize(dq * rb.rot);
    }

    // Update collider geometry
    for (auto& [id, coll] : sim.colliders) {
        if (!sim.rigidbodies.contains(id)) continue;
        coll->updateFromRigidBody(sim.rigidbodies.at(id));
    }
}
