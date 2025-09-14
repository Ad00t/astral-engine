#ifndef ENGINE_H
#define ENGINE_H

#include "opengl_includes.h"
#include "graphics/shader.h"
#include "graphics/camera.h"
#include "graphics/renderable.h" 
#include <unordered_map>
#include <vector>
#include <memory>

class GraphicsEngine {
public:
    GLFWwindow* window;
    int width, height;
    const char* title;

    GraphicsEngine(int width, int height, const char* title);
    ~GraphicsEngine();

    void addRenderable(std::shared_ptr<Renderable> r);
    void removeRenderable(std::shared_ptr<Renderable> r);
        
    void renderScene(std::shared_ptr<Camera> cam); 
   
    std::shared_ptr<Shader> getShader(const char* name); 
    void handleError(int error, const char* description);

private:
    std::unordered_map<unsigned int, std::vector<std::shared_ptr<Renderable>>> renderables;
    std::unordered_map<std::string, std::shared_ptr<Shader>> shaders;
};

#endif
