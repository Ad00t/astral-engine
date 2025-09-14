#ifndef PHYSICS_ENGINE_H
#define PHYSICS_ENGINE_H

#include <unordered_map>
#include <memory>
#include <glm/glm.hpp>

class PhysObj {
public:
    PhysObj(const glm::vec3& position = glm::vec3(0.0f),
            const glm::vec3& velocity = glm::vec3(0.0f),
            float mass = 1.0f);

    virtual ~PhysObj() = default;

    void applyForce(const glm::vec3& force);
    void integrate(float deltaTime);

    glm::vec3 position;
    glm::vec3 velocity;
    glm::vec3 acceleration;
    float mass;
};

class PhysicsEngine {
public:
    PhysicsEngine();
    ~PhysicsEngine() = default;

    void addPhysObj(int id, PhysObj*);
    void removePhysObj(int id);

    // updates all objects with Euler integration
    void updateAll(float deltaTime);

private:
    std::unordered_map<int, PhysObj*> physObjs;
};

#endif // PHYSICS_ENGINE_H
