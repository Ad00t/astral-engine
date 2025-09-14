#include "physics/physics_engine.h"
#include <algorithm>

PhysObj::PhysObj(const glm::vec3& position, const glm::vec3& velocity, float mass)
    : position(position), velocity(velocity), acceleration(0.0f), mass(mass) {}

void PhysObj::applyForce(const glm::vec3& force) {
    if (mass > 0.0f)
        acceleration += force / mass;
}

void PhysObj::integrate(float deltaTime) {
    // simple Euler integration
    velocity += acceleration * deltaTime;
    position += velocity * deltaTime;

    // reset acceleration after integration
    acceleration = glm::vec3(0.0f);
}

// ---------------- PhysicsEngine ----------------

PhysicsEngine::PhysicsEngine() {}

void PhysicsEngine::addPhysObj(int id, PhysObj* physObj) {
    physObjs.emplace(id, physObj);
}

void PhysicsEngine::removePhysObj(int id) {
    physObjs.erase(id);
}

void PhysicsEngine::updateAll(float deltaTime) {
    for (auto& [id, obj] : physObjs) {
        obj->applyForce(glm::vec3(0, -9.81, 0));
        if (obj->position.y <= 0) {
            obj->position.y = 0;
            obj->velocity.y = 0;
            obj->acceleration.y = 0;
        }
    }
    

    for (auto& [id, obj] : physObjs) {
        obj->integrate(deltaTime);
    }
}
