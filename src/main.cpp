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

int main() {
    std::shared_ptr<GraphicsEngine> gEng = std::make_shared<GraphicsEngine>("Astral Engine v1.0.0", 1600, 900);
    GUI gui(gEng->window);
    OrbitalCamera cam(gEng->window, 5e7f, 1e6f, 1e22f, 0.01f, 0.01f, 10.0f);
    std::shared_ptr<PhysicsEngine> pEng = std::make_shared<PhysicsEngine>();
    Simulation sim(gEng, pEng);

    double lastTime = glfwGetTime();
    while (!glfwWindowShouldClose(gEng->window)) {
        double now = glfwGetTime();
        double dT = now - lastTime;   // seconds since last frame
        gui.newFrame();
    
        sim.update(cam, gui.btn_paused ? 0 : dT * gui.slider_sim_speed);
    
        gui.drawElements();
        gui.render();
        gEng->finishRender();
    }

    sim.clear(); 
    cam.cleanup();
    gui.cleanup();
    gEng->cleanup();

    return 0;
}
