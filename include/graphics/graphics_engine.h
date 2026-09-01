#ifndef GRAPHICS_ENGINE_H
#define GRAPHICS_ENGINE_H

#include "opengl_includes.h"
#include "graphics/shader.h"
#include "graphics/camera.h"
#include "graphics/renderable.h" 
#include "simulation.h"
#include "update_limiter.h"
#include <string>
#include <unordered_map>
#include <memory>

class GraphicsEngine {
private:
    std::unordered_map<std::string, Renderable*> renderables;
    std::unordered_map<std::string, std::unique_ptr<Shader>> shaders;
    std::unordered_map<std::string, int> textures;

public:
    GLFWwindow* window;
    std::string title;
    std::unique_ptr<Camera> cam;
    UpdateLimiter updateLimiter;

    GraphicsEngine(std::string title, int initialWidth, int initialHeight, double maxUpdateRate);
    ~GraphicsEngine();

    void addRenderable(const std::string& id, Renderable* r);
    void removeRenderable(const std::string& id);
    void clear();

    void renderScene(); 
    void finishRender();
    void cleanup();

    // Texture key is of form <folder>/<name> for both cubemaps and uvmaps
    // e.g. uvmaps/earth_day or cubemaps/spacebox
    void loadTexture(const std::string& key);
    int getTextureID(const std::string& key);

    Shader* getShader(const std::string& key); 
    void handleError(int error, const char* description);
};

#endif
