#include "graphics/graphics_engine.h"
#include "glad/gl.h"
#include "glm/ext/matrix_transform.hpp"
#include "graphics/shader.h"
#include "graphics/camera.h"
#include "graphics/renderable.h"
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <cstdio>
#include <memory>
#include <algorithm>

GraphicsEngine::GraphicsEngine(int width, int height, const char* title):
    width(width), height(height), title(title) {
    if (!glfwInit()) {
        fprintf(stderr, "GLFW init failed\n");
        exit(EXIT_FAILURE);
    }

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    window = glfwCreateWindow(width, height, "Astral Engine", NULL, NULL);
    if (!window) {
        fprintf(stderr, "Failed to create GLFW window\n");
        glfwTerminate();
        exit(EXIT_FAILURE);
    }
    glfwMakeContextCurrent(window);

    // Load OpenGL with GLAD
    if (!gladLoadGL(glfwGetProcAddress)) {
        fprintf(stderr, "Failed to initialize GLAD\n");
        glfwDestroyWindow(window);
        glfwTerminate();
        exit(EXIT_FAILURE);
    }
    printf("OpenGL %s\n", glGetString(GL_VERSION));
    
    shaders["basic"] = std::make_shared<Shader>(
        "resources/shaders/base.vert", 
        "resources/shaders/base.frag"
    );

    printf("Graphics engine initialized\n");
}

GraphicsEngine::~GraphicsEngine() {}

void GraphicsEngine::addRenderable(std::shared_ptr<Renderable> r) {
    renderables[r->getShader()->ID].push_back(r);
}

void GraphicsEngine::removeRenderable(std::shared_ptr<Renderable> r) {
    auto it = renderables.find(r->getShader()->ID);
    if (it != renderables.end()) {
        auto &vec = it->second;
        vec.erase(std::remove(vec.begin(), vec.end(), r), vec.end());
        
        if (vec.empty()) {
            renderables.erase(it);
        }
    }
}

void GraphicsEngine::renderScene(std::shared_ptr<Camera> cam) {
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glViewport(0, 0, width, height);
    glEnable(GL_DEPTH_TEST);

    for (auto &[shaderID, shaderGroup] : renderables) {
        glUseProgram(shaderID); 
        for (auto& renderable : shaderGroup) {
            renderable->draw(cam->view, cam->projection);
        }
    }
};

std::shared_ptr<Shader> GraphicsEngine::getShader(const char* name) {
    return shaders[name];
}

void GraphicsEngine::handleError(int error, const char* description) {
    fprintf(stderr, "Graphics Engine Error %d: %s\n", error, description);
}

