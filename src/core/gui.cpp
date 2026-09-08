#include "core/gui.h"
#include "backends/imgui_impl_glfw.h"
#include "backends/imgui_impl_opengl3.h"
#include "imgui.h"
#include "core/simulation.h"
#include "utils.h"
#include <format>
#include "opengl_includes.h"

GUI::GUI(GLFWwindow* window) {
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;
    ImGui::StyleColorsDark();
    ImGui_ImplGlfw_InitForOpenGL(window, false);
    ImGui_ImplOpenGL3_Init("#version 410");
}

GUI::GUI() {

}

GUI::~GUI() {
    cleanup();
}

void GUI::newFrame() {
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();
}

void GUI::drawElements(Simulation& sim, Camera& cam) {
    ImGui::SetNextWindowPos(ImVec2(10, 10));
    ImGui::SetNextWindowBgAlpha(0.3f);
    ImGui::Begin("Options", nullptr, 
                 ImGuiWindowFlags_NoDecoration | 
                 ImGuiWindowFlags_AlwaysAutoResize |
                 ImGuiWindowFlags_NoFocusOnAppearing |
                 ImGuiWindowFlags_NoNav);
    
    ImGui::Text("FPS: %.0f", ImGui::GetIO().Framerate);
    
    if (ImGui::Button(btn_paused ? "Play" : "Pause")) {
       btn_paused = !btn_paused; 
    }
    
    ImGui::SliderFloat("Sim Speed", &slider_sim_speed, 0, 1e6f, "%.3fx", ImGuiSliderFlags_Logarithmic);

    if (ImGui::BeginCombo("Camera Target", camTargetID.c_str())) {
        for (const auto& [id, coll] : sim.colliders) {
            bool isSelected = (id == camTargetID);
            if (ImGui::Selectable(id.c_str(), isSelected)) {
                camTargetID = id;
                cam.setTarget(coll.get());
            }
            if (isSelected) {
                ImGui::SetItemDefaultFocus();
            }
        }
        ImGui::EndCombo();
    }

    ImGui::Text("Camera Radius: %s m", std::format("{:.5e}", toRealUnits(cam.radius)).c_str());
    
    ImGui::End();
}

void GUI::render() {
    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());    
}

void GUI::cleanup() {
    if (!initialized) return;
    initialized = false;
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();
}

const std::string& GUI::getCamTargetID() {
    return camTargetID;
}
