#include "physics/physics_engine.h"
#include "core/simulation.h"
#include "utils.h"
#include "glm/geometric.hpp"
#include <glm/glm.hpp>
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/quaternion.hpp>
#include <glm/gtx/norm.hpp>
#include <glm/gtx/string_cast.hpp>

PhysicsEngine::PhysicsEngine(double maxUpdateRate)
    : updateLimiter(maxUpdateRate) {}

PhysicsEngine::~PhysicsEngine() {}

void updateImpl(Simulation& sim, double dT) {
    // Gravity
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

            CollisionInfo collision = coll1->detectCollision(coll2.get());
            if (!collision.isColliding) continue;

            glm::dvec3 relative_vel = rb2.vel - rb1.vel;
            glm::dvec3 collision_normal = glm::normalize(collision.mtv);
            double constraint_speed = glm::dot(collision_normal, relative_vel);

            double total_mass = rb1.mass + rb2.mass;
            double inverse_mass = 1.0 / (1.0/rb1.mass + 1.0/rb2.mass);

            rb1.pos += collision.mtv * rb2.mass / total_mass;
            rb2.pos -= collision.mtv * rb1.mass / total_mass;

            double elasticity = coll1->restitution * coll2->restitution;
            double j_n = constraint_speed * (1.0 + elasticity) * inverse_mass;
            glm::dvec3 impulse_normal = j_n * collision_normal;

            if (constraint_speed > 0) { 
                rb1.vel += impulse_normal / rb1.mass;
                rb2.vel -= impulse_normal / rb2.mass;
            }

            glm::dvec3 v_surf_rb1 = rb1.vel + glm::cross(rb1.ang_vel, collision.contactPoint1 - rb1.pos);
            glm::dvec3 v_surf_rb2 = rb2.vel + glm::cross(rb2.ang_vel, collision.contactPoint2 - rb2.pos);
            glm::dvec3 v_surf_rel = v_surf_rb2 - v_surf_rb1;
            glm::dvec3 v_surf_rel_tan = v_surf_rel - glm::dot(v_surf_rel, collision_normal) * collision_normal;
            glm::dvec3 collision_tangential(0.0);
            double tan_len = glm::length(v_surf_rel_tan);
            if (tan_len > 1e-8) {
                collision_tangential = v_surf_rel_tan / tan_len;
            }
            double j_t_full = glm::dot(-v_surf_rel, collision_tangential) * inverse_mass;
            double mu = coll1->frictionCoeff * coll2->frictionCoeff;
            double j_t = std::clamp(j_t_full, -mu * j_n, mu * j_n);
            glm::dvec3 impulse_tangential = j_t * -collision_tangential; 

            rb1.vel += impulse_tangential / rb1.mass;
            rb2.vel -= impulse_tangential / rb2.mass;

            // rb1.ang_vel += rb1.invInertiaWorld * glm::cross(collision.contactPoint1 - rb1.pos, impulse_tangential);
            // rb2.ang_vel -= rb2.invInertiaWorld * glm::cross(collision.contactPoint2 - rb2.pos, impulse_tangential);
        }
    }
}

void PhysicsEngine::update(Simulation& sim, double dT) {
    if (!SHOULD_SUBSTEP_UPDATES || dT <= MAX_UPDATE_DT) {
        updateImpl(sim, dT);
        return;
    }

    double dTtotal = dT;
    while (dTtotal > 0) {
        dT = std::min(dTtotal, MAX_UPDATE_DT);
        updateImpl(sim, dT);
        dTtotal -= dT;
    }
}
