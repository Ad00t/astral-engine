#ifndef ENGINE_H
#define ENGINE_H

#include "opengl_includes.h"
#include "graphics/shader.h"
#include "graphics/camera.h"

class GraphicsEngine {
public:
    GLFWwindow* window;
    int width, height;
    const char* title;

    Shader baseShader;
    Shader testCubeShader;

    GLuint cubeVAO = 0;
    GLuint cubeVBO = 0;

    GraphicsEngine(int width, int height, const char* title);
    ~GraphicsEngine();

    void initCube();
    void renderScene(const Camera& cam); 
    
    void handleError(int error, const char* description);
};

#endif
