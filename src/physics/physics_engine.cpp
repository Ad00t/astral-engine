#include "physics/physics_engine.h"
#include <algorithm>

PhysicsObject::PhysicsObject(const glm::vec3& position,
                             const glm::vec3& velocity,
                             float mass)
    : position(position), velocity(velocity), acceleration(0.0f), mass(mass) {}

void PhysicsObject::applyForce(const glm::vec3& force) {
    if (mass > 0.0f)
        acceleration += force / mass;
}

void PhysicsObject::integrate(float deltaTime) {
    // simple Euler integration
    velocity += acceleration * deltaTime;
    position += velocity * deltaTime;

    // reset acceleration after integration
    acceleration = glm::vec3(0.0f);
}

// ---------------- PhysicsEngine ----------------

PhysicsEngine::PhysicsEngine() = default;

void PhysicsEngine::addPhysObj(std::shared_ptr<PhysicsObject> physObj) {
    physObjs.push_back(physObj);
}

void PhysicsEngine::removePhysObj(std::shared_ptr<PhysicsObject> physObj) {
    physObjs.erase(std::remove(physObjs.begin(), physObjs.end(), physObj));
}

void PhysicsEngine::updateAll(float deltaTime) {
    for (auto obj : physObjs) {
        obj->applyForce(glm::vec3(0, -9.81, 0));
        if (obj->position.y <= 0) {
            obj->position.y = 0;
            obj->velocity.y = 0;
            obj->acceleration.y = 0;
        }
    }
    

    for (auto& obj : physObjs) {
        obj->integrate(deltaTime);
    }
}
