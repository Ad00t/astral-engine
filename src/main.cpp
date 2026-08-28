#include "GLFW/glfw3.h"
#include "opengl_includes.h"
#include "graphics/graphics_engine.h"
#include "graphics/camera.h"
#include "physics/physics_engine.h"
#include "simulation.h"
#include "utils.h"
#include "gui.h"
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

std::mutex state_mutex;
std::atomic<bool> run_sim;
std::thread sim_thread;

void sim_thread_func() {
    auto last_update_time = std::chrono::steady_clock::now();
    while (run_sim.load()) {
        auto frame_start = std::chrono::steady_clock::now();
        double dT = std::chrono::duration<double>(frame_start - last_update_time).count();
        last_update_time = frame_start;
        
        std::unique_lock<std::mutex> lock(state_mutex);
        sim->update(gui->btn_paused ? 0 : dT * gui->slider_sim_speed);
        lock.unlock();

        auto frame_end = std::chrono::steady_clock::now();
        double elapsed = std::chrono::duration<double>(frame_end - frame_start).count();
        if (elapsed < 1.0/SIM_MAX_FPS) {
            std::this_thread::sleep_for(std::chrono::duration<double>(1.0/SIM_MAX_FPS - elapsed));
        }
    }
}

int main() {
    gEng = std::make_shared<GraphicsEngine>("Astral Engine v1.0.0", 1600, 900);
    pEng = std::make_shared<PhysicsEngine>();
    sim = std::make_shared<Simulation>(gEng, pEng);
    gui = std::make_shared<GUI>(gEng->window);

    gEng->cam->target = sim->getSimObj(1);
   
    run_sim.store(true);
    sim_thread = std::thread(sim_thread_func);

    while (!glfwWindowShouldClose(gEng->window)) {
        auto frame_start = std::chrono::steady_clock::now();
        
        gui->newFrame();

        std::unique_lock<std::mutex> lock(state_mutex);
        gEng->renderScene();
        lock.unlock();

        gui->drawElements();
        gui->render();
        gEng->finishRender();

        auto frame_end = std::chrono::steady_clock::now();
        double elapsed = std::chrono::duration<double>(frame_end - frame_start).count();
        if (elapsed < 1.0/GRAPHICS_MAX_FPS) {
            std::this_thread::sleep_for(std::chrono::duration<double>(1.0/GRAPHICS_MAX_FPS- elapsed));
        }
    }

    run_sim.store(false);
    if (sim_thread.joinable()) {
        sim_thread.join();
    }

    sim->clear(); 
    gui->cleanup();
    gEng->cleanup();

    return 0;
}
