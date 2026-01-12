#include "simulation.h"
#include "graphics/camera.h"
#include "graphics/graphics_engine.h"
#include "graphics/renderable.h"
#include "physics/physics_engine.h"
#include "utils.h"
#include <glm/gtc/matrix_transform.hpp>
#include <memory>

SimObj::SimObj(int id, std::unique_ptr<Renderable> renderable, std::unique_ptr<PhysObj> physObj)
    : id(id), renderable(std::move(renderable)), physObj(std::move(physObj)) {}

void SimObj::syncPhysicsToRender() {
    glm::mat4 model = glm::translate(glm::mat4(1.0f), toRender(physObj->pos));
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

Simulation::Simulation(std::shared_ptr<GraphicsEngine> gEng, std::shared_ptr<PhysicsEngine> pEng)
    : gEng(gEng), pEng(pEng) {
    addSimObj(0, // Sun 
        std::make_unique<Sphere>(gEng, glm::vec3(1, 1, 0), 6.957e8),
        std::make_unique<PhysObj>(glm::vec3(0, 0, 0), glm::vec3(0, 0, 0), 1.989e30)
    );

    addSimObj(1, // Earth 
        std::make_unique<Sphere>(gEng, glm::vec3(0, 0, 1), 6.371e6),
        std::make_unique<PhysObj>(glm::vec3(1.496e11, 0, 0), glm::vec3(0, 3.0e4, 0), 5.972e24)
    );
    
    addSimObj(2, // Moon 
        std::make_unique<Sphere>(gEng, glm::vec3(1, 1, 1), 1.7375e6),
        std::make_unique<PhysObj>(glm::vec3(1.496e11 + 3.84e8, 0, 0), glm::vec3(0, 3.0e4 + 1.022e3, 0), 7.35e22)
    );
}

Simulation::~Simulation() {
    clear();
}

void Simulation::addSimObj(int id, std::unique_ptr<Renderable> renderable, std::unique_ptr<PhysObj> physObj) {
    SimObj obj(id, std::move(renderable), std::move(physObj));
    gEng->addRenderable(id, obj.getRenderable());
    pEng->addPhysObj(id, obj.getPhysObj());
    simObjs.emplace(id, std::move(obj));
}

void Simulation::removeSimObj(int id) {
    pEng->removePhysObj(id);
    gEng->removeRenderable(id);
    simObjs.erase(id);
}

const SimObj* Simulation::getSimObj(int id) const {
    auto it = simObjs.find(id);
    if (it != simObjs.end())
        return &it->second;  
    return nullptr;        
}

void Simulation::clear() {
    pEng->clear();
    gEng->clear();
    simObjs.clear();
}

void Simulation::update(OrbitalCamera& cam, float deltaTime) {
    pEng->updateAll(deltaTime);
    for (auto& [id, simObj] : simObjs) {
        simObj.syncPhysicsToRender();
    }
    cam.update(getSimObj(1)->getPhysObj()->pos);
    gEng->renderScene(cam);
}
