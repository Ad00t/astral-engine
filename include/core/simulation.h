#ifndef SIMULATION_H
#define SIMULATION_H

#include "graphics/renderable.h"
#include "physics/rigidbody.h"
#include "physics/collider.h"
#include <unordered_map>
#include <memory>
#include <atomic>

class GraphicsEngine;
class PhysicsEngine;

class Simulation {
public:
    std::mutex stateMutex;
    std::atomic<bool> runPhysics;

    std::unordered_map<std::string, std::unique_ptr<Renderable>> renderables;
    std::unordered_map<std::string, RigidBody> rigidbodies;
    std::unordered_map<std::string, std::unique_ptr<Collider>> colliders;

public:
    Simulation(GraphicsEngine& gEng, PhysicsEngine& pEng);
    ~Simulation();

    void syncPhysicsUpdate();
    void clear();
};

#endif // SIMULATION_H
