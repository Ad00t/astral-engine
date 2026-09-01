#include "simulation.h"
#include "graphics/camera.h"
#include "graphics/graphics_engine.h"
#include "graphics/renderable.h"
#include "physics/physics_engine.h"
#include "utils.h"
#include <glm/gtc/matrix_transform.hpp>
#include <memory>

SimObj::SimObj(const std::string& id, std::unique_ptr<Renderable> renderable, std::unique_ptr<PhysObj> physObj)
    : id(id), renderable(std::move(renderable)), physObj(std::move(physObj)) {}

void SimObj::syncPhysicsToRender() {
    if (getPhysObj() == nullptr) return;
    glm::mat4 model = glm::translate(glm::mat4(1.0f), toRender(physObj->pos));
    renderable->setModel(model);
}

std::string SimObj::getID() const {
    return id;
}

Renderable* SimObj::getRenderable() const {
    return renderable.get();
}

PhysObj* SimObj::getPhysObj() const {
    return physObj.get();
}

// ---------------- SIMULATION -----------------

Simulation::Simulation(std::shared_ptr<GraphicsEngine> gEng, std::shared_ptr<PhysicsEngine> pEng, double maxUpdateRate)
    : gEng(gEng), pEng(pEng), updateLimiter(maxUpdateRate) {

    addSimObj("spacebox", // Spacebox
        std::make_unique<SkyBox>(gEng, glm::vec3(0, 0, 0), "cubemap/spacebox"),
        std::unique_ptr<PhysObj>(nullptr)
    );

    addSimObj("sun", // Sun 
        std::make_unique<Sphere>(gEng, glm::vec3(1, 1, 0), "uvmap/sun", 7.957e8),
        std::make_unique<PhysObj>(glm::vec3(0, 0, 0), glm::vec3(0, 0, 0), 1.989e30)
    );

    addSimObj("earth", // Earth 
        std::make_unique<Sphere>(gEng, glm::vec3(0, 0, 1), "uvmap/earth_day", 6.371e6),
        std::make_unique<PhysObj>(glm::vec3(1.496e11, 0, 0), glm::vec3(0, 3.0e4, 0), 5.972e24)
    );
    
    addSimObj("moon", // Moon
        std::make_unique<Sphere>(gEng, glm::vec3(1, 1, 1), "uvmap/moon", 1.7375e6),
        std::make_unique<PhysObj>(glm::vec3(1.496e11 + 3.84e8, 0, 0), glm::vec3(0, 3.0e4 + 1.022e3, 0), 7.35e22)
    );

    pEng->computeForces();
    for (auto& [id, simObj] : simObjs) {
        if (simObj.getPhysObj() == nullptr) continue;
        simObj.getPhysObj()->acc = simObj.getPhysObj()->acc_new;
    }
    for (auto& [id, simObj] : simObjs) {
        if (simObj.getPhysObj() == nullptr) continue;
        simObj.getPhysObj()->acc_new = glm::dvec3(0.0);
    }
}

Simulation::Simulation() {}
Simulation::~Simulation() {
    clear();
}

void Simulation::addSimObj(const std::string& id, std::unique_ptr<Renderable> renderable, std::unique_ptr<PhysObj> physObj) {
    SimObj obj(id, std::move(renderable), std::move(physObj));
    if (obj.getRenderable() != nullptr)
        gEng->addRenderable(id, obj.getRenderable());
    if (obj.getPhysObj() != nullptr)
        pEng->addPhysObj(id, obj.getPhysObj());
    simObjs.emplace(id, std::move(obj));
}

void Simulation::removeSimObj(const std::string& id) {
    pEng->removePhysObj(id);
    gEng->removeRenderable(id);
    simObjs.erase(id);
}

const SimObj* Simulation::getSimObj(const std::string& id) const {
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

void Simulation::update(float deltaTime) {
    pEng->updateAll(deltaTime);
    for (auto& [id, simObj] : simObjs) {
        simObj.syncPhysicsToRender();
    }
}
