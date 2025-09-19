#include "graphics/camera.h" 
#include "glm/ext/matrix_clip_space.hpp"
#include "imgui_impl_glfw.h"
#include "opengl_includes.h"
#include <GLFW/glfw3.h>
#define GLM_ENABLE_EXPERIMENTAL
#include "glm/gtx/string_cast.hpp"
#include "utils.h"
#include "imgui.h"
#include <cmath>
#include <iostream>
#include <string>

Camera::Camera(GLFWwindow* window, glm::vec3 position = glm::vec3(0.0f), float orbitSpeed = 0.01f, float panSpeed = 0.01f, float zoomSpeed = 1.0f)
    : window(window), position(position), orbitSpeed(orbitSpeed), panSpeed(panSpeed), zoomSpeed(zoomSpeed) {
    glfwGetWindowSize(window, &width, &height);
    glfwSetWindowUserPointer(window, this);
    glfwSetMouseButtonCallback(window, [](GLFWwindow* win, int button, int action, int mods) {
        Camera* cam = (Camera*)glfwGetWindowUserPointer(win);
        ImGuiIO& io = ImGui::GetIO();
        ImGui_ImplGlfw_MouseButtonCallback(win, button, action, mods);
        if (!io.WantCaptureMouse) 
            cam->handleMouseButton(win, button, action, mods);
    });
    glfwSetCursorPosCallback(window, [](GLFWwindow* win, double x, double y) {
        Camera* cam = (Camera*)glfwGetWindowUserPointer(win);
        ImGuiIO& io = ImGui::GetIO();
        ImGui_ImplGlfw_CursorPosCallback(win, x, y);
        if (!io.WantCaptureMouse) 
            cam->handleMouseMove(win, x, y);
    });
    glfwSetScrollCallback(window, [](GLFWwindow* win, double xoffset, double yoffset) {
        Camera* cam = (Camera*)glfwGetWindowUserPointer(win);
        ImGuiIO& io = ImGui::GetIO();
        ImGui_ImplGlfw_ScrollCallback(win, xoffset, yoffset);
        if (!io.WantCaptureMouse) 
            cam->handleMouseScroll(win, xoffset, yoffset);
    });
    glfwSetKeyCallback(window, [](GLFWwindow* win, int key, int scancode, int action, int mods) {
        Camera* cam = (Camera*)glfwGetWindowUserPointer(win);
        ImGuiIO& io = ImGui::GetIO();
        ImGui_ImplGlfw_KeyCallback(win, key, scancode, action, mods);
        if (!io.WantCaptureKeyboard) 
            cam->handleKeyboard(win, key, scancode, action, mods);
    });
    glfwSetFramebufferSizeCallback(window, [](GLFWwindow* win, int width, int height) {
        Camera* cam = (Camera*)glfwGetWindowUserPointer(win);
        glViewport(0, 0, width, height);
        glfwGetWindowSize(win, &cam->width, &cam->height);
    });
}

Camera::~Camera() {
    cleanup();
}
void Camera::update() {}
void Camera::cleanup() {
    glfwSetMouseButtonCallback(window, nullptr);
    glfwSetCursorPosCallback(window, nullptr);
    glfwSetScrollCallback(window, nullptr);
    glfwSetKeyCallback(window, nullptr);
    glfwSetFramebufferSizeCallback(window, nullptr);
}

void Camera::handleMouseMove(GLFWwindow* win, double x, double y) {}
void Camera::handleMouseButton(GLFWwindow* win, int button, int action, int mods) {}
void Camera::handleMouseScroll(GLFWwindow* win, double xoffset, double yoffset) {}
void Camera::handleKeyboard(GLFWwindow* win, int key, int scancode, int action, int mods) {}

OrbitalCamera::OrbitalCamera(GLFWwindow* window, double initialRealRadius, double minRealRadius, double maxRealRadius, float orbitSpeed, float panSpeed, float zoomSpeed)
    : Camera(window, glm::vec3(0.0f), orbitSpeed, panSpeed, zoomSpeed), radius(toRender(initialRealRadius)), minRadius(toRender(minRealRadius)), maxRadius(toRender(maxRealRadius)) {}

OrbitalCamera::~OrbitalCamera() {}

void OrbitalCamera::update() {
    update(glm::vec3(0.0f));
}

void OrbitalCamera::update(glm::dvec3 realTarget) {
    glm::vec3 renderTarget = toRender(realTarget);

    position = glm::vec3(
        radius * cos(elevation) * cos(azimuth),
        radius * cos(elevation) * sin(azimuth),
        radius * sin(elevation)
    ) + renderTarget;
    // position = toRender(glm::dvec3(1.496e11 + 5e7f, 0, 5e7f)); 
    
    model = glm::mat4(1.0f);
    view = glm::lookAt(position, renderTarget, glm::vec3(0,0,1));
    projection = glm::infinitePerspective(glm::radians(60.0f), float(width) / float(height), 0.1f);
}

void OrbitalCamera::handleMouseMove(GLFWwindow* win, double x, double y) {
    float dx = float(x - lastX);
    float dy = float(y - lastY);

    if (dragging) {
        azimuth = std::fmod(azimuth + dx*orbitSpeed, 2*M_PI);
        elevation = glm::clamp(elevation - dy*orbitSpeed, -float(M_PI)/2 + 0.01f, float(M_PI)/2 - 0.01f);
        // printf("dx:%.1f dy:%.1f az:%.2f el:%.2f\n", dx, dy, glm::degrees(azimuth), glm::degrees(elevation)); 
    }

    lastX = x;
    lastY = y;
}

void OrbitalCamera::handleMouseButton(GLFWwindow* win, int button, int action, int mods) {
    if (button == GLFW_MOUSE_BUTTON_LEFT) {
        if (action == GLFW_PRESS) {
            dragging = true;
            glfwGetCursorPos(win, &lastX, &lastY);
        } else if (action == GLFW_RELEASE) {
            dragging = false;
        }
    }
}

void OrbitalCamera::handleMouseScroll(GLFWwindow* win, double xoffset, double yoffset) { 
    radius = glm::clamp(radius - (float) yoffset*zoomSpeed, minRadius, maxRadius);
}
