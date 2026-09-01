#ifndef SIMULATION_H
#define SIMULATION_H

#include "graphics/renderable.h"
#include "physics/physics_engine.h"
#include "update_limiter.h"
#include <unordered_map>
#include <memory>

class GraphicsEngine;

class SimObj {
private:
    std::string id;
    std::unique_ptr<Renderable> renderable;
    std::unique_ptr<PhysObj> physObj;

public:
    SimObj(const std::string& id, std::unique_ptr<Renderable> renderable, std::unique_ptr<PhysObj> physObj);
    ~SimObj() = default;

    // default move operations
    SimObj(SimObj&&) = default;
    SimObj& operator=(SimObj&&) = default;

    // delete copy operations
    SimObj(const SimObj&) = delete;
    SimObj& operator=(const SimObj&) = delete;

    // update renderable model from physObj 
    void syncPhysicsToRender();

    // accessors
    std::string getID() const;
    Renderable* getRenderable() const;
    PhysObj* getPhysObj() const;
};

class Simulation {
private:
    std::shared_ptr<GraphicsEngine> gEng;
    std::shared_ptr<PhysicsEngine> pEng;
    std::unordered_map<std::string, SimObj> simObjs;

public:
    UpdateLimiter updateLimiter;

    Simulation(std::shared_ptr<GraphicsEngine> gEng, std::shared_ptr<PhysicsEngine> pEng, double maxUpdateRate);
    Simulation();
    ~Simulation();

    void addSimObj(const std::string& id, std::unique_ptr<Renderable> renderable, std::unique_ptr<PhysObj> physObj);
    void removeSimObj(const std::string& id);
    const SimObj* getSimObj(const std::string& id) const;
    void clear();

    // step physObjs and sync to renderables
    void update(float deltaTime);
};

#endif // SIMULATION_H
