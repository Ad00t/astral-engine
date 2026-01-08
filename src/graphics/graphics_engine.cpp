#include "graphics/graphics_engine.h"
#include "glad/gl.h"
#include "graphics/shader.h"
#include "graphics/camera.h"
#include "graphics/renderable.h"
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <cstdio>
#include <memory>
#include <algorithm>

GraphicsEngine::GraphicsEngine(std::string title, int initialWidth, int initialHeight)
    : title(title) {
    if (!glfwInit()) {
        fprintf(stderr, "GLFW init failed\n");
        exit(EXIT_FAILURE);
    }

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    window = glfwCreateWindow(initialWidth, initialHeight, title.c_str(), NULL, NULL);
    if (!window) {
        fprintf(stderr, "Failed to create GLFW window\n");
        glfwTerminate();
        exit(EXIT_FAILURE);
    }
    glfwMakeContextCurrent(window);
    glfwSwapInterval(1); // Enable v-sync

    // Load OpenGL with GLAD
    if (!gladLoadGL(glfwGetProcAddress)) {
        fprintf(stderr, "Failed to initialize GLAD\n");
        glfwDestroyWindow(window);
        glfwTerminate();
        exit(EXIT_FAILURE);
    }
    printf("OpenGL %s\n", glGetString(GL_VERSION));
    
    shaders["basic"] = std::make_unique<Shader>(
        "resources/shaders/base.vert", 
        "resources/shaders/base.frag"
    );

    glViewport(0, 0, initialWidth, initialHeight);
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    glEnable(GL_DEPTH_TEST);
    printf("Graphics engine initialized\n");
}

GraphicsEngine::~GraphicsEngine() {
    cleanup();
}

void GraphicsEngine::addRenderable(int id, Renderable* r) {
    renderables.emplace(id, r);
    shaderGroups[r->getShader()->ID].push_back(id);
}

void GraphicsEngine::removeRenderable(int id) {
    auto it = shaderGroups.find(renderables[id]->getShader()->ID);
    if (it != shaderGroups.end()) {
        auto &vec = it->second;
        vec.erase(std::remove(vec.begin(), vec.end(), id), vec.end());
        if (vec.empty()) {
            shaderGroups.erase(it);
        }
    }
    renderables.erase(id);
}

void GraphicsEngine::clear() {
    shaderGroups.clear();
    renderables.clear();
}

void GraphicsEngine::renderScene(const Camera& cam) {
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    for (auto& [shaderID, group] : shaderGroups) {
        glUseProgram(shaderID); 
        for (auto& id : group) {
            renderables[id]->draw(cam.view, cam.projection);
        }
    }
};

void GraphicsEngine::finishRender() {
    glfwPollEvents();
    glfwSwapBuffers(window);
}

void GraphicsEngine::cleanup() {
    glfwDestroyWindow(window);
    glfwTerminate();
}

Shader* GraphicsEngine::getShader(const std::string& name) {
    auto it = shaders.find(name);
    if (it == shaders.end()) {
        return nullptr; 
    }
    return it->second.get();
}
void GraphicsEngine::handleError(int error, const char* description) {
    fprintf(stderr, "Graphics Engine Error %d: %s\n", error, description);
}

