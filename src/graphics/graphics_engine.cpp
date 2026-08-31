#include "graphics/graphics_engine.h"
#include "glad/gl.h"
#include "graphics/shader.h"
#include "graphics/camera.h"
#include "graphics/renderable.h"
#include "update_limiter.h"
#include "graphics/stb_image.h"
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <cstdio>
#include <memory>
#if defined(_WIN32)
#include <windows.h>
#include <timeapi.h>
#pragma comment(lib, "winmm.lib")
#endif

GraphicsEngine::GraphicsEngine(std::string title, int initialWidth, int initialHeight, double maxUpdateRate)
    : title(title), updateLimiter(maxUpdateRate) {
    if (!glfwInit()) {
        fprintf(stderr, "GLFW init failed\n");
        exit(EXIT_FAILURE);
    }

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 1);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
#ifdef __APPLE__
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif

    window = glfwCreateWindow(initialWidth, initialHeight, title.c_str(), NULL, NULL);
    if (!window) {
        fprintf(stderr, "Failed to create GLFW window\n");
        glfwTerminate();
        exit(EXIT_FAILURE);
    }
    glfwMakeContextCurrent(window);
    glfwSwapInterval(0); 

    // Load OpenGL with GLAD
    if (!gladLoadGL(glfwGetProcAddress)) {
        fprintf(stderr, "Failed to initialize GLAD\n");
        glfwDestroyWindow(window);
        glfwTerminate();
        exit(EXIT_FAILURE);
    }
    printf("OpenGL %s\n", glGetString(GL_VERSION));
   
    // Load shaders
    shaders["simobj"] = std::make_unique<Shader>("simobj.vert", "simobj.frag");

    // Load textures
    // stbi_set_flip_vertically_on_load(true);
    loadTexture("sun_tex.jpg");
    loadTexture("earth_tex.jpg");
    loadTexture("moon_tex.jpg");

    int fbWidth, fbHeight;
    glfwGetFramebufferSize(window, &fbWidth, &fbHeight);
    glViewport(0, 0, fbWidth, fbHeight);
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    glEnable(GL_DEPTH_TEST);

#if defined(_WIN32)
    timeBeginPeriod(1); // Sets the minimum OS sleep granularity to 1ms
#endif

    cam = std::make_unique<Camera>(window, 5e7f, 1e6f, 1e22f, 0.01f, 0.01f, 10.0f);
    printf("Graphics engine initialized\n");
}

GraphicsEngine::~GraphicsEngine() {
    cleanup();
}

void GraphicsEngine::addRenderable(int id, Renderable* r) {
    renderables.emplace(id, r);
}

void GraphicsEngine::removeRenderable(int id) {
    renderables.erase(id);
}

void GraphicsEngine::clear() {
    renderables.clear();
}

void GraphicsEngine::renderScene() {
    cam->update();
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    for (auto& [id, r] : renderables) {
        r->draw(cam->view, cam->projection);
    }
}

void GraphicsEngine::finishRender() {
    glfwPollEvents();
    glfwSwapBuffers(window);
}

void GraphicsEngine::cleanup() {
    cam->cleanup();
#if defined(_WIN32)
    timeEndPeriod(1); // Clean up before exiting
#endif
    glfwDestroyWindow(window);
    glfwTerminate();
}

void GraphicsEngine::loadTexture(const std::string& file) {
    uint32_t texture;
    glGenTextures(1, &texture);
    glBindTexture(GL_TEXTURE_2D, texture);
    // set the texture wrapping/filtering options (on the currently bound texture object)
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);	
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    // load and generate the texture
    int width, height, nrChannels;
    std::string fp = "resources/assets/" + file;
    uint8_t* data = stbi_load(fp.c_str(), &width, &height, &nrChannels, 0);
    if (data) {
        GLenum format = (nrChannels == 4) ? GL_RGBA : (nrChannels == 1 ? GL_RED : GL_RGB);
        glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
        glGenerateMipmap(GL_TEXTURE_2D);
    } else {
        printf("Failed to load texture: '%s'\n", file.c_str());
    }
    stbi_image_free(data);
    textures.emplace(file, texture);
}

int GraphicsEngine::getTextureID(const std::string& file) {
    auto it = textures.find(file);
    if (it == textures.end()) {
        return -1; 
    }
    return it->second;
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

