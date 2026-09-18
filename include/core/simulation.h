#ifndef SIMULATION_H
#define SIMULATION_H

#include "graphics/renderable.h"
#include "physics/rigidbody.h"
#include "physics/collider.h"
#include "core/controller.h"
#include <unordered_map>
#include <memory>
#include <atomic>

class GraphicsEngine;
class PhysicsEngine;

class Entity {
public:
    std::string id;
    std::unique_ptr<RigidBody> rigidbody;
    std::unique_ptr<Controller> controller;
    std::unordered_map<std::string, std::unique_ptr<Renderable>> renderables;
    std::unordered_map<std::string, std::unique_ptr<Collider>> colliders;

    Entity(const std::string& id);
    ~Entity() = default;

    void addRenderable(std::unique_ptr<Renderable> rend);
    void addCollider(std::unique_ptr<Collider> coll);
};

class Simulation {
public:
    std::mutex stateMutex;
    std::atomic<bool> runPhysics;
    std::unordered_map<std::string, std::unique_ptr<Entity>> entities;

public:
    Simulation(GraphicsEngine& gEng, PhysicsEngine& pEng);
    ~Simulation() = default;

    void addEntity(std::unique_ptr<Entity> entity);
    void updateFromPhysics();
};

#endif // SIMULATION_H
