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
    std::string title;

    GraphicsEngine(int width, int height, std::string title);
    ~GraphicsEngine();

    void addRenderable(int id, Renderable* r);
    void removeRenderable(int id);
        
    void renderScene(const Camera& cam); 
   
    Shader* getShader(std::string name); 
    void handleError(int error, const char* description);

private:
    std::unordered_map<int, Renderable*> renderables;
    std::unordered_map<int, std::vector<int>> shaderGroups;
    std::unordered_map<std::string, std::unique_ptr<Shader>> shaders;
};

#endif
