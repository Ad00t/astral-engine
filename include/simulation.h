#ifndef SIMULATION_H
#define SIMULATION_H

#include "graphics/graphics_engine.h"
#include "graphics/camera.h"
#include "graphics/renderable.h"
#include "physics/physics_engine.h"
#include <vector>
#include <memory>

class SimObj {
public:
    SimObj(std::shared_ptr<Renderable> renderable,
           std::shared_ptr<PhysicsObject> physObj);

    ~SimObj() = default;

    // update renderable model from physObj 
    void syncPhysicsToRender();

    // accessors
    std::shared_ptr<Renderable> getRenderable() const;
    std::shared_ptr<PhysicsObject> getPhysObj() const;

private:
    std::shared_ptr<Renderable> renderable;
    std::shared_ptr<PhysicsObject> physObj;
};

class Simulation {
public:
    Simulation(std::shared_ptr<GraphicsEngine> gEng,
               std::shared_ptr<PhysicsEngine> pEng);

    ~Simulation() = default;

    // add an object to simulation
    void addSimObj(std::shared_ptr<SimObj> obj);

    // remove an object from simulation
    void removeSimObj(std::shared_ptr<SimObj> obj);

    // main update loop: steps physObj, syncs objects, and renders
    void update(std::shared_ptr<Camera> cam, float deltaTime);

private:
    std::shared_ptr<GraphicsEngine> gEng;
    std::shared_ptr<PhysicsEngine> pEng;
    std::vector<std::shared_ptr<SimObj>> simObjs;
};

#endif // SIMULATION_H
