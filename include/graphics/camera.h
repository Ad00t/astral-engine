#ifndef CAMERA_H
#define CAMERA_H

#include "opengl_includes.h"
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <cmath>

class Camera {
public:
    int width, height;

    float orbitSpeed;
    float panSpeed;
    float zoomSpeed;

    GLFWwindow* window;
    glm::vec3 position;
    glm::mat4 model;
    glm::mat4 view;
    glm::mat4 projection; 

    Camera(GLFWwindow* window, glm::vec3 position, float orbitSpeed, float panSpeed, float zoomSpeed);
    ~Camera();

    virtual void update();
    void cleanup();

    virtual void handleMouseMove(GLFWwindow* win, double x, double y);
    virtual void handleMouseButton(GLFWwindow* win, int button, int action, int mods);
    virtual void handleMouseScroll(GLFWwindow* win, double xoffset, double yoffset);
    virtual void handleKeyboard(GLFWwindow* win, int key, int scancode, int action, int mods);
};

class OrbitalCamera : public Camera {
public:
    float radius;
    float minRadius, maxRadius;

    bool dragging = false;
    double lastX = 0.0, lastY = 0.0;
   
    float azimuth = 0.0f;
    float elevation = 0.0f;

    OrbitalCamera(GLFWwindow* window, double initialRealRadius, double minRealRadius, double maxRealRadius, float orbitSpeed, float panSpeed, float zoomSpeed);
    ~OrbitalCamera();

    void update() override;
    void update(glm::dvec3 realTarget);

    void handleMouseMove(GLFWwindow* win, double x, double y) override;
    void handleMouseButton(GLFWwindow* win, int button, int action, int mods) override;
    void handleMouseScroll(GLFWwindow* win, double xoffset, double yoffset) override;
};

#endif
