#ifndef CAMERA_H
#define CAMERA_H

#include "physics/collider.h"
#include "opengl_includes.h"
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

class Renderable;

class Camera {
public:
    int width, height;

    float orbitSpeed;
    float panSpeed;
    float zoomSpeed;

    GLFWwindow* window;
    glm::dvec3 realPos;
    glm::vec3 renderPos;
    glm::mat4 model;
    glm::mat4 view;
    glm::mat4 projection; 

    float radius;
    float minRadius, maxRadius;

    bool dragging = false;
    double lastX = 0.0, lastY = 0.0;
   
    float azimuth = 0.0f;
    float elevation = 0.0f;

    Collider* target;

    Camera(GLFWwindow* window, double initialRealRadius, double minRealRadius, double maxRealRadius, float orbitSpeed, float panSpeed, float zoomSpeed);
    ~Camera();
    Camera(const Camera&) = delete;

    void setTarget(Collider* newTarget);
    void update();
    void cleanup();

    void handleMouseMove(GLFWwindow* win, double x, double y);
    void handleMouseButton(GLFWwindow* win, int button, int action, int mods);
    void handleMouseScroll(GLFWwindow* win, double xoffset, double yoffset);
    void handleKeyboard(GLFWwindow* win, int key, int scancode, int action, int mods);
};

#endif
