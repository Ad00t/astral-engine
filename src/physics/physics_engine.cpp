#include "physics/physics_engine.h"
#include "glm/glm.hpp" 
#define GLM_ENABLE_EXPERIMENTAL
#include "glm/gtx/string_cast.hpp"
#include <cstdio>
#include <iostream>
#include <algorithm>
#include <cmath>

PhysObj::PhysObj(glm::vec3 pos, glm::vec3 vel, float mass)
    : pos(pos), vel(vel), acc(0.0f), mass(mass) {}

void PhysObj::applyForce(const glm::vec3& force) {
    if (mass > 0.0f)
        acc += force / mass;
}

void PhysObj::integrate(float dT) {
    // Velocity Verlet integration 
    pos = pos + vel*dT + 0.5f*acc*powf(dT, 2);
    vel = vel + acc*dT;

    // Reset acc after integration
    acc = glm::vec3(0.0f);
}

// ---------------- PhysicsEngine ----------------

PhysicsEngine::PhysicsEngine() {}

void PhysicsEngine::addPhysObj(int id, PhysObj* physObj) {
    physObjs.emplace(id, physObj);
}

void PhysicsEngine::removePhysObj(int id) {
    physObjs.erase(id);
}

void PhysicsEngine::clear() {
    physObjs.clear();
}

// Main physics loop logic
void PhysicsEngine::updateAll(float dT) {
    for (auto it1 = physObjs.begin(); it1 != physObjs.end(); ++it1) {
        auto& [id1, obj1] = *it1;
        auto it2 = it1;
        ++it2;
        for (auto it2 = it1; it2 != physObjs.end(); ++it2) {
            auto& [id2, obj2] = *it2;
            glm::vec3 dir = obj2->pos - obj1->pos;
            float r = glm::length(dir);
            if (r < 1e-4f) continue;
            glm::vec3 F_g = (float)(G * obj1->mass * obj2->mass / powf(r, 2)) * glm::normalize(dir);
            obj1->applyForce(F_g);
            obj2->applyForce(-F_g);
        } 
    }
    

    for (auto& [id, obj] : physObjs) {
        obj->integrate(dT);
    }
}
