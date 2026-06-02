#include "physics/physics_engine.h"
#include "glm/glm.hpp" 
#define GLM_ENABLE_EXPERIMENTAL
#include "glm/gtx/string_cast.hpp"
#include <cstdio>
#include <iostream>
#include <algorithm>
#include <cmath>

constexpr double G = 6.67430e-11;

// ---------------- PhysObj ----------------

PhysObj::PhysObj(glm::dvec3 pos, glm::dvec3 vel, double mass)
    : pos(pos), vel(vel), acc(glm::dvec3(0.0)), acc_new(glm::dvec3(0.0)), mass(mass) {}

void PhysObj::applyForce(const glm::dvec3& force) {
    if (mass > 0.0)
        acc_new += force / mass;
}

void PhysObj::integratePos(double dT) {
    pos = pos + vel * dT + 0.5 * acc * dT * dT;
}

void PhysObj::integrateVel(double dT) {
    vel = vel + 0.5 * (acc + acc_new) * dT;
    acc = acc_new;
    acc_new = glm::dvec3(0.0);
}

// ---------------- PhysicsEngine ----------------

PhysicsEngine::PhysicsEngine() {}

PhysicsEngine::~PhysicsEngine() {
    clear();
}

void PhysicsEngine::addPhysObj(int id, PhysObj* physObj) {
    physObjs.emplace(id, physObj);
}

void PhysicsEngine::removePhysObj(int id) {
    physObjs.erase(id);
}

void PhysicsEngine::clear() {
    physObjs.clear();
}

void PhysicsEngine::computeForces() {
    for (auto it1 = physObjs.begin(); it1 != physObjs.end(); ++it1) {
        auto& [id1, obj1] = *it1;
        auto it2 = it1;
        ++it2;
        for (; it2 != physObjs.end(); ++it2) {
            auto& [id2, obj2] = *it2;

            glm::dvec3 dir = obj2->pos - obj1->pos;
            double r = glm::length(dir);
            if (r < 1e-4) continue;
            glm::dvec3 F_g = (G * obj1->mass * obj2->mass / (r * r)) * glm::normalize(dir);

            obj1->applyForce(F_g);
            obj2->applyForce(-F_g);
        }
    }
}

void PhysicsEngine::updateAll(float dT) {
    // 1. Update positions using current acc
    for (auto& [id, obj] : physObjs)
        obj->integratePos(dT);

    // 2. Compute forces at new positions → acc_new
    computeForces();

    // 3. Update velocities using (acc + acc_new) / 2, then swap
    for (auto& [id, obj] : physObjs)
        obj->integrateVel(dT);
}
