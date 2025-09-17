#include "opengl_includes.h"
#include "graphics/graphics_engine.h"
#include "graphics/camera.h"
#include "physics/physics_engine.h"
#include "simulation.h"
#include "utils.h"
#include "gui.h"
#include "imgui.h"
#include <cstdio>
#include <memory>

int main() {
    GraphicsEngine gEng(1920, 1080, "Astral Engine v1.0.0");
    GUI gui(gEng.window);
    OrbitalCamera cam(gEng.window, 5e7f, 1e6f, 1e22f, 0.01f, 0.01f, 10.0f);
    PhysicsEngine pEng;
    Simulation sim(gEng, pEng);

    sim.addSimObj(0, // Sun 
        std::make_unique<Sphere>(gEng.getShader("basic"), glm::vec3(1, 1, 0), 6.957e8),
        std::make_unique<PhysObj>(glm::vec3(0, 0, 0), glm::vec3(0, 0, 0), 1.989e30)
    );

    sim.addSimObj(1, // Earth 
        std::make_unique<Sphere>(gEng.getShader("basic"), glm::vec3(0, 0, 1), 6.371e6),
        std::make_unique<PhysObj>(glm::vec3(1.496e11, 0, 0), glm::vec3(0, 0, 3.0e4), 5.972e24)
    );
    
    sim.addSimObj(2, // Moon 
        std::make_unique<Sphere>(gEng.getShader("basic"), glm::vec3(1, 1, 1), 1.7375e6),
        std::make_unique<PhysObj>(glm::vec3(1.496e11 + 3.84e8, 0, 0), glm::vec3(0, 0, 3.0e4 + 1.022e3), 7.35e22)
    );

    int targetID = 2;
    double lastTime = glfwGetTime();
    
    while (!glfwWindowShouldClose(gEng.window)) {
        double now = glfwGetTime();
        double dT = now - lastTime;   // seconds since last frame
        lastTime = now;
        gui.newFrame();
     
        cam.update(sim.getSimObj(1)->getPhysObj()->pos);
        sim.update(cam, gui.btn_paused ? 0 : dT * gui.slider_sim_speed);
               
        gui.drawElements();
        gui.render();
        gEng.finishRender();
    }

    sim.clear(); 
    cam.cleanup();
    gui.cleanup();
    gEng.cleanup();

    return 0;
}
