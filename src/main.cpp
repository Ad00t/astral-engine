#include "opengl_includes.h"
#include "graphics/graphics_engine.h"
#include "graphics/camera.h"
#include "imgui.h"
#include "backends/imgui_impl_glfw.h"
#include "backends/imgui_impl_opengl3.h"
#include "physics/physics_engine.h"
#include "simulation.h"
#include <cstdio>
#include <memory>

int main() {
    GraphicsEngine gEng(1280, 720, "Astral Engine");
    OrbitalCamera cam(gEng.window);
    PhysicsEngine pEng;
    Simulation sim(gEng, pEng);

    sim.addSimObj(1, 
        std::make_unique<Cube>(gEng.getShader("basic"), glm::vec3(0,0,1)),
        std::make_unique<PhysObj>(glm::vec3(0, 10, 0))
    );

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;
    ImGui::StyleColorsDark();
    ImGui_ImplGlfw_InitForOpenGL(gEng.window, true);
    ImGui_ImplOpenGL3_Init("#version 430");

    double lastTime = glfwGetTime();

    while (!glfwWindowShouldClose(gEng.window)) {
        double now = glfwGetTime();
        double dt = now - lastTime;   // seconds since last frame
        lastTime = now;

        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();
        
        cam.update();
        sim.update(cam, dt);

        ImGui::SetNextWindowPos(ImVec2(10, 10));
        ImGui::SetNextWindowBgAlpha(0.3f); // Transparent background
        ImGui::Begin("Statistics", nullptr, ImGuiWindowFlags_NoDecoration | 
                                            ImGuiWindowFlags_AlwaysAutoResize |
                                            ImGuiWindowFlags_NoFocusOnAppearing |
                                            ImGuiWindowFlags_NoNav);
        ImGui::Text("FPS: %.1f", ImGui::GetIO().Framerate);
        
        ImGui::End();
        ImGui::Render();
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());    

        glfwSwapBuffers(gEng.window);
        glfwPollEvents();
    }

    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();
    glfwDestroyWindow(gEng.window);
    glfwTerminate();

    return 0;
}
