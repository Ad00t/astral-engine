#ifndef PHYSICS_ENGINE_H
#define PHYSICS_ENGINE_H

#include <vector>
#include <memory>
#include <glm/glm.hpp>

class PhysicsObject {
public:
    PhysicsObject(const glm::vec3& position = glm::vec3(0.0f),
                  const glm::vec3& velocity = glm::vec3(0.0f),
                  float mass = 1.0f);

    virtual ~PhysicsObject() = default;

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

    void addPhysObj(std::shared_ptr<PhysicsObject> physObj);
    void removePhysObj(std::shared_ptr<PhysicsObject> physObj);

    // updates all objects with Euler integration
    void updateAll(float deltaTime);

private:
    std::vector<std::shared_ptr<PhysicsObject>> physObjs;
};

#endif // PHYSICS_ENGINE_H
