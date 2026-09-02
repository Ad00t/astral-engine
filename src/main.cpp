#include "GLFW/glfw3.h"
#include "opengl_includes.h"
#include "graphics/graphics_engine.h"
#include "physics/physics_engine.h"
#include "simulation.h"
#include "utils.h"
#include "gui.h"
#include "update_limiter.h"
#include <memory>
#include <thread>
#include <chrono>
#include <mutex>
#include <atomic>

#define PHYSICS_MAX_FPS         120
#define GRAPHICS_MAX_FPS        240

std::unique_ptr<GraphicsEngine> gEng;
std::unique_ptr<PhysicsEngine> pEng;
std::unique_ptr<Simulation> sim;
std::unique_ptr<GUI> gui;
std::thread physicsThread;

void physicsThreadFunc() {
    auto lastUpdateTime = std::chrono::steady_clock::now();
    while (sim->runPhysics.load()) {
        pEng->updateLimiter.startUpdate();

        double dT = std::chrono::duration<double>(pEng->updateLimiter.updateStart - lastUpdateTime).count();
        dT = gui->btn_paused ? 0 : dT * gui->slider_sim_speed;
        lastUpdateTime = pEng->updateLimiter.updateStart;
        
        std::unique_lock<std::mutex> lock(sim->stateMutex);
        pEng->updateRigidBodies(sim->rigidbodies, dT);
        sim->syncPhysicsToRender();
        lock.unlock();

        pEng->updateLimiter.endUpdate();
    }
}

int main() {
    gEng = std::make_unique<GraphicsEngine>("Astral Engine v1.0.0", 1600, 900, GRAPHICS_MAX_FPS); 
    pEng = std::make_unique<PhysicsEngine>(PHYSICS_MAX_FPS);
    sim = std::make_unique<Simulation>(*gEng, *pEng);
    gui = std::make_unique<GUI>(gEng->window);

    gEng->cam->setTarget(sim->renderables.at(gui->getCamTargetID()).get());
    physicsThread = std::thread(physicsThreadFunc);

    while (!glfwWindowShouldClose(gEng->window)) {
        gEng->updateLimiter.startUpdate(); 

        gui->newFrame();

        std::unique_lock<std::mutex> lock(sim->stateMutex);
        gEng->renderScene(sim->renderables);
        gui->drawElements(sim->rigidbodies, sim->renderables, *gEng->cam);
        lock.unlock();

        gui->render();
        gEng->finishRender();

        gEng->updateLimiter.endUpdate();
    }

    // Cleanup
    sim->runPhysics.store(false);
    if (physicsThread.joinable()) {
        physicsThread.join();
    }
    sim->clear();
    gui->cleanup();
    return 0;
}
