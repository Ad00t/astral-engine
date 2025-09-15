#include "simulation.h"
#include "graphics/graphics_engine.h"
#include "graphics/renderable.h"
#include "physics/physics_engine.h"
#include <algorithm>
#include <glm/gtc/matrix_transform.hpp>
#include <memory>

SimObj::SimObj(int id, std::unique_ptr<Renderable> renderable, std::unique_ptr<PhysObj> physObj)
    : id(id), renderable(std::move(renderable)), physObj(std::move(physObj)) {}

void SimObj::syncPhysicsToRender() {
    glm::mat4 model = glm::translate(glm::mat4(1.0f), physObj->pos * (float)SCALE_FACTOR);
    renderable->setModel(model);
}

int SimObj::getID() const {
    return id;
}

Renderable* SimObj::getRenderable() const {
    return renderable.get();
}

PhysObj* SimObj::getPhysObj() const {
    return physObj.get();
}

// ---------------- SIMULATION -----------------

Simulation::Simulation(GraphicsEngine& gEng, PhysicsEngine& pEng)
    : gEng(gEng), pEng(pEng) {}

void Simulation::addSimObj(int id, std::unique_ptr<Renderable> renderable, std::unique_ptr<PhysObj> physObj) {
    SimObj obj(id, std::move(renderable), std::move(physObj));
    gEng.addRenderable(id, obj.getRenderable());
    pEng.addPhysObj(id, obj.getPhysObj());
    simObjs.emplace(id, std::move(obj));
}

void Simulation::removeSimObj(int id) {
    pEng.removePhysObj(id);
    gEng.removeRenderable(id);
    simObjs.erase(id);
}

const SimObj* Simulation::getSimObj(int id) const {
    auto it = simObjs.find(id);
    if (it != simObjs.end())
        return &it->second;  
    return nullptr;        
}

void Simulation::update(const Camera& cam, float deltaTime) {
    pEng.updateAll(deltaTime);
    for (auto& [id, simObj] : simObjs) {
        simObj.syncPhysicsToRender();
    }
    gEng.renderScene(cam);
}
