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
    std::unordered_map<std::string, Shader> shaders;
    std::unordered_map<std::string, GLuint> textures;

    GLuint hdrFBO = 0;
    GLuint atmoFBO = 0;
    GLuint bloomFBO[2];

    GLuint hdrColorTex[2];
    GLuint hdrDepthTex = 0;
    GLuint bloomColorTex[2];

    int width = 0, height = 0;
    GLuint quadVAO = 0, quadVBO = 0;

    void createRenderTargets(int width, int height);
    void destroyRenderTargets();
    void resizeRenderTargets(int width, int height);
    void setupScreenQuad();

public:
    GLFWwindow* window;
    std::string title;
    std::unique_ptr<Camera> cam;
    UpdateLimiter updateLimiter;

    GraphicsEngine(std::string title, int initialWidth, int initialHeight, double maxUpdateRate);
    ~GraphicsEngine();

    void renderScene(Simulation& sim); 
    void finishRender();
    void cleanup();

    // Texture key is of form <folder>/<name> for both cubemaps and uvmaps
    // e.g. uvmaps/earth_day or cubemaps/spacebox
    void loadTexture(const std::string& key);
    GLuint& getTextureID(const std::string& key);

    Shader& getShader(const std::string& key); 
    void handleError(int error, const char* description);
};

#endif
