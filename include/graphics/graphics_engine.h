#ifndef GRAPHICS_ENGINE_H
#define GRAPHICS_ENGINE_H

#include "opengl_includes.h"
#include "graphics/shader.h"
#include "graphics/camera.h"
#include "graphics/renderable.h" 
#include "simulation.h"
#include <string>
#include <unordered_map>
#include <memory>

class GraphicsEngine {
private:
    std::unordered_map<int, Renderable*> renderables;
    std::unordered_map<std::string, std::unique_ptr<Shader>> shaders;

public:
    GLFWwindow* window;
    std::string title;
    std::unique_ptr<Camera> cam;

    GraphicsEngine(std::string title, int initialWidth, int initialHeight);
    ~GraphicsEngine();

    void addRenderable(int id, Renderable* r);
    void removeRenderable(int id);
    void clear();

    void renderScene(); 
    void finishRender();
    void cleanup();

    Shader* getShader(const std::string& name); 
    void handleError(int error, const char* description);
};

#endif
