#include "simulation.h"
#include "graphics/graphics_engine.h"
#include "graphics/renderable.h"
#include "physics/physics_engine.h"
#include <algorithm>
#include <glm/gtc/matrix_transform.hpp>

SimObj::SimObj(std::shared_ptr<Renderable> renderable,
               std::shared_ptr<PhysicsObject> physObj)
    : renderable(renderable), physObj(physObj) {}

void SimObj::syncPhysicsToRender() {
    if (physObj && renderable) {
        // simple translation based on physObj position
        glm::mat4 model = glm::translate(glm::mat4(1.0f), physObj->position);
        renderable->setModel(model);
    }
}

std::shared_ptr<Renderable> SimObj::getRenderable() const {
    return renderable;
}

std::shared_ptr<PhysicsObject> SimObj::getPhysObj() const {
    return physObj;
}
Simulation::Simulation(std::shared_ptr<GraphicsEngine> gEng,
                       std::shared_ptr<PhysicsEngine> pEng)
    : gEng(gEng), pEng(pEng) {}

// ---------------- SIMULATION -----------------

void Simulation::addSimObj(std::shared_ptr<SimObj> obj) {
    simObjs.push_back(obj);
    gEng->addRenderable(obj->getRenderable());
    pEng->addPhysObj(obj->getPhysObj());
}

void Simulation::removeSimObj(std::shared_ptr<SimObj> obj) {
    pEng->removePhysObj(obj->getPhysObj());
    gEng->removeRenderable(obj->getRenderable());
    simObjs.erase(std::remove(simObjs.begin(), simObjs.end(), obj), simObjs.end());
}

void Simulation::update(std::shared_ptr<Camera> cam, float deltaTime) {
    pEng->updateAll(deltaTime);
    for (auto &obj : simObjs) {
        obj->syncPhysicsToRender();
    }
    gEng->renderScene(cam);
}
