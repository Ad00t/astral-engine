#include "physics/physics_engine.h"
#include "core/entity_controller.h"
#include "core/simulation.h"
#include "glm/ext/vector_double3.hpp"
#include "physics/rigidbody.h"
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

void PhysicsEngine::updateImpl(Simulation& sim, double dt) {
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

            rb1.applyForceAtPoint(F_g, rb1.pos, true);
            rb2.applyForceAtPoint(F_g, rb2.pos, true);
        }
    }
   
    // Integrate velocities & angular velocities
    for (auto& [id, rb] : sim.rigidbodies) {
        rb.vel = rb.vel + 0.5 * (rb.acc + rb.acc_new) * dt;
        rb.acc = rb.acc_new;
        rb.acc_new = glm::dvec3(0.0);

        rb.ang_vel = rb.ang_vel + 0.5 * (rb.ang_acc + rb.ang_acc_new) * dt;
        rb.ang_acc = rb.ang_acc_new;
        rb.ang_acc_new = glm::dvec3(0.0);
    }

    // Integrate positions & rotations
    for (auto& [id, rb] : sim.rigidbodies) {
        rb.pos = rb.pos + rb.vel * dt + 0.5 * rb.acc * dt*dt;

        double w_len = glm::length(rb.ang_vel);
        if (w_len > 1e-12) {
            glm::dvec3 axis = rb.ang_vel / w_len;
            glm::quat dq = glm::angleAxis(w_len * dt, axis);
            rb.rot = glm::normalize(dq * rb.rot);
        }
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

            // Separate bodies

            double total_mass = rb1.mass + rb2.mass;
            rb1.pos += collision.mtv * rb2.mass / total_mass;
            rb2.pos -= collision.mtv * rb1.mass / total_mass;

            glm::dvec3 r1 = collision.contactPoint1 - rb1.pos;
            glm::dvec3 r2 = collision.contactPoint2 - rb2.pos;
            glm::dvec3 v_cp_rb1 = rb1.vel + glm::cross(rb1.ang_vel, r1);
            glm::dvec3 v_cp_rb2 = rb2.vel + glm::cross(rb2.ang_vel, r2);
            glm::dvec3 v_cp_rel = v_cp_rb2 - v_cp_rb1;

            glm::dvec3 collision_n = glm::normalize(collision.mtv);
            double constraint_speed = glm::dot(collision_n, v_cp_rel);
            if (constraint_speed <= 1e-12) continue;

            // Normal impulse
            
            glm::dmat3 invI1 = rb1.getInvInertiaWorld();
            glm::dmat3 invI2 = rb2.getInvInertiaWorld();

            double ang_term_n =
                glm::dot(collision_n, glm::cross(invI1 * glm::cross(r1, collision_n), r1)) +
                glm::dot(collision_n, glm::cross(invI2 * glm::cross(r2, collision_n), r2));
            double k_n = 1.0/rb1.mass + 1.0/rb2.mass + ang_term_n;

            double elasticity = coll1->restitution * coll2->restitution;
            double j_n = (constraint_speed * (1.0 + elasticity)) / k_n;
            glm::dvec3 impulse_n = j_n * collision_n;

            rb1.vel += impulse_n / rb1.mass;
            rb2.vel -= impulse_n / rb2.mass;
            rb1.ang_vel += invI1 * glm::cross(r1, impulse_n);
            rb2.ang_vel -= invI2 * glm::cross(r2, impulse_n);

            // Recompute surface velocities after the normal impulse for friction

            v_cp_rb1 = rb1.vel + glm::cross(rb1.ang_vel, r1);
            v_cp_rb2 = rb2.vel + glm::cross(rb2.ang_vel, r2);
            v_cp_rel = v_cp_rb2 - v_cp_rb1;
            glm::dvec3 v_cp_rel_t = v_cp_rel - glm::dot(v_cp_rel, collision_n) * collision_n;

            // Tangential (friction) impulse

            glm::dvec3 collision_t(0.0);
            double speed_cp_rel_t = glm::length(v_cp_rel_t);
            if (speed_cp_rel_t > 1e-8) {
                collision_t = v_cp_rel_t / speed_cp_rel_t;
            }

            double ang_term_t =
                glm::dot(collision_t, glm::cross(invI1 * glm::cross(r1, collision_t), r1)) +
                glm::dot(collision_t, glm::cross(invI2 * glm::cross(r2, collision_t), r2));
            double k_t = 1.0/rb1.mass + 1.0/rb2.mass + ang_term_t;

            double j_t_full = glm::dot(-v_cp_rel, collision_t) / k_t;
            double mu = coll1->frictionCoeff * coll2->frictionCoeff;
            double j_t = std::clamp(j_t_full, -mu * j_n, mu * j_n);
            glm::dvec3 impulse_t = j_t * -collision_t;
            
            rb1.vel += impulse_t / rb1.mass;
            rb2.vel -= impulse_t / rb2.mass;
            // rb1.ang_vel += invI1 * glm::cross(r1, impulse_t);
            // rb2.ang_vel -= invI2 * glm::cross(r2, impulse_t);
        }
    }

    // Update controllers
    for (auto& [id, con] : sim.controllers) {
        if (!sim.rigidbodies.contains(id)) continue;
        RigidBody& rb = sim.rigidbodies.at(id);
        con->updateSensors(rb);
        con->runControlLoop(rb, dt);
        rb.g_acc = glm::dvec3(0.0);
    }
}

void PhysicsEngine::update(Simulation& sim, double dt) {
    if (!config.should_substep_updates || dt <= config.max_update_dt) {
        updateImpl(sim, dt);
        return;
    }

    double dt_total = dt;
    while (dt_total > 0) {
        dt = std::min(dt_total, config.max_update_dt);
        updateImpl(sim, dt);
        dt_total -= dt;
    }
}
