#ifndef SIMULATION_H
#define SIMULATION_H

#include "graphics/graphics_engine.h"
#include "graphics/camera.h"
#include "graphics/renderable.h"
#include "physics/physics_engine.h"
#include <unordered_map>
#include <memory>

class SimObj {
public:
    SimObj(int id, std::unique_ptr<Renderable> renderable, std::unique_ptr<PhysObj> physObj);
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
    int getID() const;
    Renderable* getRenderable() const;
    PhysObj* getPhysObj() const;

private:
    int id;
    std::unique_ptr<Renderable> renderable;
    std::unique_ptr<PhysObj> physObj;
};

class Simulation {
public:
    Simulation(GraphicsEngine& gEng, PhysicsEngine& pEng);
    ~Simulation();

    void addSimObj(int id, std::unique_ptr<Renderable> renderable, std::unique_ptr<PhysObj> physObj);
    void removeSimObj(int id);
    const SimObj* getSimObj(int id) const;
    void clear();

    // main update loop: steps physObj, syncs objects, and renders
    void update(const Camera& cam, float deltaTime);

private:
    GraphicsEngine& gEng;
    PhysicsEngine& pEng;
    std::unordered_map<int, SimObj> simObjs;
};

#endif // SIMULATION_H
