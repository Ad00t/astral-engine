#include "graphics/camera.h" 
#include "graphics/renderable.h"
#include "core/simulation.h"
#include "utils.h"
#include "glm/ext/matrix_clip_space.hpp"
#include "imgui_impl_glfw.h"
#include "opengl_includes.h"
#define GLM_ENABLE_EXPERIMENTAL
#include "glm/gtx/string_cast.hpp"
#include "imgui.h"
#include <cmath>

Camera::Camera(GLFWwindow* window, double initialRealRadius, double minRealRadius, double maxRealRadius, float orbitSpeed, float panSpeed, float zoomSpeed)
    : window(window), radius(toRenderUnits(initialRealRadius)), minRadius(toRenderUnits(minRealRadius)), maxRadius(toRenderUnits(maxRealRadius)),
     orbitSpeed(orbitSpeed), panSpeed(panSpeed), zoomSpeed(zoomSpeed), realPos(glm::dvec3(0.0)), renderPos(glm::vec3(0.0f)), target(nullptr) {

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
        if (width == 0 || height == 0) return;
        Camera* cam = (Camera*)glfwGetWindowUserPointer(win);
        cam->width = width;
        cam->height = height;
    });
}

Camera::~Camera() {
    cleanup();
}

void Camera::setTarget(Renderable* newTarget) {
    target = newTarget;
    minRadius = 1.5f * target->renderScale;
    maxRadius = 1000.0f * target->renderScale;
    radius = glm::clamp(minRadius * 5.0f, minRadius, maxRadius);
}

static glm::mat4 infinitePerspectiveReversedZ(float fovY, float aspect, float zNear) {
    float f = 1.0f / tanf(fovY * 0.5f);
    glm::mat4 m(0.0f);
    m[0][0] = f / aspect;
    m[1][1] = f;
    m[2][2] = 0.0f;     // Maps infinity to exactly 0.0
    m[2][3] = -1.0f;
    m[3][2] = zNear;    // Maps near plane to 1.0
    return m;
}

void Camera::update() {
    if (target == nullptr) return;

    radius = glm::clamp(radius, minRadius, maxRadius);
    glm::dvec3 realOffset = toRealUnits(glm::vec3(
        radius * cos(elevation) * cos(azimuth),
        radius * cos(elevation) * sin(azimuth),
        radius * sin(elevation)
    ));
    glm::dvec3 camRealPos = target->realPos + realOffset;
    realPos = camRealPos;

    // double subtraction first (safe cancellation), then scale+cast down
    glm::vec3 relTargetPos = toRenderUnits(target->realPos - camRealPos);
    view = glm::lookAt(glm::vec3(0.0f), relTargetPos, glm::vec3(0,0,1));

    float dynamicNear = glm::max(radius * 0.001f, 0.01f);
    minRadius = glm::max(target->renderScale * 1.2f, dynamicNear * 2.0f);
    projection = infinitePerspectiveReversedZ(glm::radians(60.0f), float(width) / float(height), dynamicNear);
}

void Camera::cleanup() {
    glfwSetMouseButtonCallback(window, nullptr);
    glfwSetCursorPosCallback(window, nullptr);
    glfwSetScrollCallback(window, nullptr);
    glfwSetKeyCallback(window, nullptr);
    glfwSetFramebufferSizeCallback(window, nullptr);
}

void Camera::handleMouseMove(GLFWwindow* win, double x, double y) {
    float dx = float(x - lastX);
    float dy = float(y - lastY);

    if (dragging) {
        azimuth = std::fmod(azimuth + dx*orbitSpeed, 2*M_PI);
        elevation = glm::clamp(elevation - dy*orbitSpeed, -float(M_PI)/2 + 0.01f, float(M_PI)/2 - 0.01f);
    }

    lastX = x;
    lastY = y;
}

void Camera::handleMouseButton(GLFWwindow* win, int button, int action, int mods) {
    if (button == GLFW_MOUSE_BUTTON_LEFT) {
        if (action == GLFW_PRESS) {
            dragging = true;
            glfwGetCursorPos(win, &lastX, &lastY);
        } else if (action == GLFW_RELEASE) {
            dragging = false;
        }
    }
}

void Camera::handleMouseScroll(GLFWwindow* win, double xoffset, double yoffset) { 
    float zoomFactor = powf(1.0f - zoomSpeed, (float) yoffset);
    radius = glm::clamp(radius * zoomFactor, minRadius, maxRadius);
}

void Camera::handleKeyboard(GLFWwindow* win, int key, int scancode, int action, int mods) {

}
