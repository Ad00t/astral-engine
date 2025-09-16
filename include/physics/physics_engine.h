#ifndef PHYSICS_ENGINE_H
#define PHYSICS_ENGINE_H

#include <unordered_map>
#include <memory>
#include <glm/glm.hpp>

#define G   6.67430E-11

class PhysObj {
public:
    glm::vec3 pos;      // m
    glm::vec3 vel;      // m/s
    glm::vec3 acc;      // m/s^2
    float mass;         // kg

    PhysObj(glm::vec3 pos = glm::vec3(0.0f),
            glm::vec3 vel = glm::vec3(0.0f),
            float mass = 1.0f);

    virtual ~PhysObj() = default;

    void applyForce(const glm::vec3& force);
    void integrate(float deltaTime);
};

class PhysicsEngine {
private:
    std::unordered_map<int, PhysObj*> physObjs;

public:
    PhysicsEngine();
    ~PhysicsEngine() = default;

    void addPhysObj(int id, PhysObj*);
    void removePhysObj(int id);
    void clear();

    // updates physics state of all objects 
    void updateAll(float dT);
};

#endif // PHYSICS_ENGINE_H
