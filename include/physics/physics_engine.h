#ifndef PHYSICS_ENGINE_H
#define PHYSICS_ENGINE_H

#include <unordered_map>
#include <memory>
#include <glm/glm.hpp>

class PhysObj {
public:
    glm::dvec3 pos;      // m
    glm::dvec3 vel;      // m/s
    glm::dvec3 acc;      // m/s^2
    glm::dvec3 acc_new;  // m/s^2
    double mass;         // kg

    PhysObj(glm::dvec3 pos = glm::dvec3(0.0),
            glm::dvec3 vel = glm::dvec3(0.0),
            double mass = 1.0);

    virtual ~PhysObj() = default;

    void applyForce(const glm::dvec3& force);
    void integrate(double dT);
};

class PhysicsEngine {
private:
    std::unordered_map<int, PhysObj*> physObjs;

public:
    PhysicsEngine();
    ~PhysicsEngine();

    void addPhysObj(int id, PhysObj*);
    void removePhysObj(int id);
    void clear();

    // updates physics state of all objects 
    void updateAll(float dT);
};

#endif // PHYSICS_ENGINE_H
