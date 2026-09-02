#ifndef SIMULATION_H
#define SIMULATION_H

#include "graphics/renderable.h"
#include "physics/rigidbody.h"
#include "graphics/graphics_engine.h"
#include "physics/physics_engine.h"
#include <unordered_map>
#include <memory>
#include <atomic>

class GraphicsEngine;

class Simulation {
public:
    std::mutex stateMutex;
    std::atomic<bool> runPhysics;

    std::unordered_map<std::string, std::unique_ptr<Renderable>> renderables;
    std::unordered_map<std::string, RigidBody> rigidbodies;
    std::unordered_map<std::string, Material> materials;

public:
    Simulation(GraphicsEngine& gEng, PhysicsEngine& pEng);
    ~Simulation();

    void syncPhysicsToRender();
    void clear();
};

#endif // SIMULATION_H
