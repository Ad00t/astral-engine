// References: https://github.com/kavan010/black_hole/blob/main/black_hole.cpp

#include "graphics/camera.h" 
#include "glm/ext/matrix_clip_space.hpp"
#include "opengl_includes.h"
#define GLM_ENABLE_EXPERIMENTAL
#include "glm/gtx/string_cast.hpp"
#include <iostream>
#include <string>

Camera::Camera(GLFWwindow* window, glm::vec3 position = glm::vec3(0.0f), float orbitSpeed = 0.01f, float panSpeed = 0.01f, float zoomSpeed = 1.0f)
    : window(window), position(position), orbitSpeed(orbitSpeed), panSpeed(panSpeed), zoomSpeed(zoomSpeed) {
    glfwGetWindowSize(window, &width, &height);
    glfwSetWindowUserPointer(window, this);
    glfwSetMouseButtonCallback(window, [](GLFWwindow* win, int button, int action, int mods) {
        Camera* cam = (Camera*)glfwGetWindowUserPointer(win);
        cam->handleMouseButton(win, button, action, mods);
    });
    glfwSetCursorPosCallback(window, [](GLFWwindow* win, double x, double y) {
        Camera* cam = (Camera*)glfwGetWindowUserPointer(win);
        cam->handleMouseMove(win, x, y);
    });
    glfwSetScrollCallback(window, [](GLFWwindow* win, double xoffset, double yoffset) {
        Camera* cam = (Camera*)glfwGetWindowUserPointer(win);
        cam->handleMouseScroll(win, xoffset, yoffset);
    });
    glfwSetKeyCallback(window, [](GLFWwindow* win, int key, int scancode, int action, int mods) {
        Camera* cam = (Camera*)glfwGetWindowUserPointer(win);
        cam->handleKeyboard(win, key, scancode, action, mods);
    });
}
Camera::~Camera() {}

void Camera::update() {}
void Camera::handleMouseMove(GLFWwindow* win, double x, double y) {}
void Camera::handleMouseButton(GLFWwindow* win, int button, int action, int mods) {}
void Camera::handleMouseScroll(GLFWwindow* win, double xoffset, double yoffset) {}
void Camera::handleKeyboard(GLFWwindow* win, int key, int scancode, int action, int mods) {}

OrbitalCamera::OrbitalCamera(GLFWwindow* window, float initialRadius, float minRadius, float maxRadius, float orbitSpeed, float panSpeed, float zoomSpeed)
    : Camera(window, glm::vec3(0.0f), orbitSpeed, panSpeed, zoomSpeed), radius(initialRadius), minRadius(minRadius), maxRadius(maxRadius) {}
OrbitalCamera::~OrbitalCamera() {}

void OrbitalCamera::update() {
    update(glm::vec3(0.0f));
}

void OrbitalCamera::update(glm::vec3 target) {
    if (dragging || panning) {
        moving = true;
    } else {
        moving = false;
    }

    float clampedElevation = glm::clamp(elevation, 0.01f, float(M_PI) - 0.01f);
    
    position = glm::vec3(
        radius * sin(clampedElevation) * cos(azimuth),
        radius * cos(clampedElevation),
        radius * sin(clampedElevation) * sin(azimuth)
    ) + target;
   
    model = glm::mat4(1.0f);
    view = glm::lookAt(position, target, glm::vec3(0,1,0));
    projection = glm::infinitePerspective(glm::radians(60.0f), float(width) / float(height), minRadius * 1e-2f);
}

void OrbitalCamera::handleMouseMove(GLFWwindow* win, double x, double y) {
    float dx = float(x - lastX);
    float dy = float(y - lastY);

    if (dragging && panning) {
        // Pan: Shift + Left or Middle Mouse
        // Disable panning to keep camera centered on black hole
    } else if (dragging && !panning) {
        // Orbit: Left mouse only
        azimuth   += dx * orbitSpeed;
        elevation -= dy * orbitSpeed;
        elevation = glm::clamp(elevation, 0.01f, float(M_PI) - 0.01f);
    }

    lastX = x;
    lastY = y;
}

void OrbitalCamera::handleMouseButton(GLFWwindow* win, int button, int action, int mods) {
    if (button == GLFW_MOUSE_BUTTON_LEFT || button == GLFW_MOUSE_BUTTON_MIDDLE) {
        if (action == GLFW_PRESS) {
            dragging = true;
            // Disable panning so camera always orbits center
            panning = false;
            glfwGetCursorPos(win, &lastX, &lastY);
        } else if (action == GLFW_RELEASE) {
            dragging = false;
            panning = false;
        }
    }
}

void OrbitalCamera::handleMouseScroll(GLFWwindow* win, double xoffset, double yoffset) {
    radius -= yoffset * zoomSpeed;
    radius = glm::clamp(radius, minRadius, maxRadius);
}
