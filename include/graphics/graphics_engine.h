#ifndef GRAPHICS_ENGINE_H
#define GRAPHICS_ENGINE_H

#include "opengl_includes.h"
#include "graphics/shader.h"
#include "graphics/camera.h"
#include "graphics/renderable.h" 
#include "core/simulation.h"
#include "core/update_limiter.h"
#include <string>
#include <unordered_map>
#include <memory>

constexpr int BLOOM_DOWNSAMPLE = 2;
constexpr int NUM_BLOOM_PASSES = 6;
constexpr bool MSAA_ENABLED = true;
constexpr int MSAA_SAMPLES = 16;

class GraphicsEngine {
private:
    std::unordered_map<std::string, Shader> shaders;
    std::unordered_map<std::string, GLuint> textures;

    GLuint msaaFBO = 0;
    GLuint hdrFBO = 0;
    GLuint atmoFBO = 0;
    GLuint bloomFBO[2];

    GLuint msaaColorTex[2];
    GLuint msaaDepthTex = 0;
    GLuint hdrColorTex[2];
    GLuint hdrDepthTex = 0;
    GLuint bloomColorTex[2];

    int width = 0, height = 0;
    int bloomWidth, bloomHeight;
    
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
    static GLuint loadTextureFromFile(const std::string& path, GLenum wrapMode = GL_REPEAT);
    static GLuint loadTextureFromMemory(const uint8_t* data, size_t size, GLenum wrapMode = GL_REPEAT);
    static GLuint loadTextureFromRawRGBA(const uint8_t* data, int width, int height);
    static GLuint loadTextureCubemap(const std::string& path);

    GLuint& getTextureID(const std::string& key);

    Shader& getShader(const std::string& key); 
    void handleError(int error, const char* description);
};

#endif
