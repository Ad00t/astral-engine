#include "GLFW/glfw3.h"
#include "opengl_includes.h"
#include "graphics/graphics_engine.h"
#include "graphics/camera.h"
#include "physics/physics_engine.h"
#include "simulation.h"
#include "utils.h"
#include "gui.h"
#include "update_limiter.h"
#include <cstdio>
#include <memory>
#include <thread>
#include <chrono>
#include <mutex>
#include <atomic>

#define SIM_MAX_FPS             120
#define GRAPHICS_MAX_FPS        240

std::shared_ptr<GraphicsEngine> gEng;
std::shared_ptr<PhysicsEngine> pEng;
std::shared_ptr<Simulation> sim;
std::shared_ptr<GUI> gui;

std::mutex stateMutex;
std::atomic<bool> runSim;
std::thread simThread;

void simThreadFunc() {
    auto lastUpdateTime = std::chrono::steady_clock::now();
    while (runSim.load()) {
        sim->updateLimiter.startUpdate();
        double dT = std::chrono::duration<double>(sim->updateLimiter.updateStart - lastUpdateTime).count();
        lastUpdateTime = sim->updateLimiter.updateStart;
        
        std::unique_lock<std::mutex> lock(stateMutex);
        sim->update(gui->btn_paused ? 0 : dT * gui->slider_sim_speed);
        lock.unlock();

        sim->updateLimiter.endUpdate();
    }
}

int main() {
    gEng = std::make_shared<GraphicsEngine>("Astral Engine v1.0.0", 1600, 900, GRAPHICS_MAX_FPS);
    pEng = std::make_shared<PhysicsEngine>();
    sim = std::make_shared<Simulation>(gEng, pEng, SIM_MAX_FPS);
    gui = std::make_shared<GUI>(gEng->window);

    gEng->cam->target = sim->getSimObj(1);
   
    runSim.store(true);
    simThread = std::thread(simThreadFunc);

    while (!glfwWindowShouldClose(gEng->window)) {
        gEng->updateLimiter.startUpdate(); 
        gui->newFrame();

        std::unique_lock<std::mutex> lock(stateMutex);
        gEng->renderScene();
        lock.unlock();

        gui->drawElements();
        gui->render();
        gEng->finishRender();
        gEng->updateLimiter.endUpdate();
    }

    runSim.store(false);
    if (simThread.joinable()) {
        simThread.join();
    }

    sim->clear(); 
    gui->cleanup();
    gEng->cleanup();

    return 0;
}
