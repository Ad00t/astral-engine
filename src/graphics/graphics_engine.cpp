#include "graphics/graphics_engine.h"
#include "glad/gl.h"
#include "graphics/shader.h"
#include "graphics/camera.h"
#include "graphics/renderable.h"
#include "update_limiter.h"
#include "graphics/stb_image.h"
#include "utils.h"
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
    glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, GLFW_TRUE);
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

#ifndef __APPLE__
    glEnable(GL_DEBUG_OUTPUT);
    glDebugMessageCallback([](GLenum, GLenum, GLuint, GLenum severity, GLsizei, const GLchar* msg, const void*) {
        if (severity != GL_DEBUG_SEVERITY_NOTIFICATION)
            fprintf(stderr, "GL: %s\n", msg);
    }, nullptr);
#endif
   
    // Load shaders
    shaders["entity"] = Shader("entity.vert", "entity.frag");
    shaders["skybox"] = Shader("skybox.vert", "skybox.frag");
    shaders["atmosphere"] = Shader("atmosphere.vert", "atmosphere.frag");
    shaders["tonemap"] = Shader("tonemap.vert", "tonemap.frag");

    // Load textures
    // stbi_set_flip_vertically_on_load(true);
    loadTexture("uvmap/earth_day");
    loadTexture("uvmap/earth_night");
    loadTexture("uvmap/sun");
    loadTexture("uvmap/moon");
    loadTexture("cubemap/spacebox");

    int width, height; 
    glfwGetFramebufferSize(window, &width, &height);
    glViewport(0, 0, width, height);
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_TEXTURE_CUBE_MAP_SEAMLESS);

#if defined(_WIN32)
    timeBeginPeriod(1); // Sets the minimum OS sleep granularity to 1ms
#endif

    cam = std::make_unique<Camera>(window, 5e7f, 1e6f, 1e22f, 0.01f, 0.01f, 10.0f);

    createHDRFramebuffer(width, height);
    setupScreenQuad();

    printf("Graphics engine initialized\n");
}

GraphicsEngine::~GraphicsEngine() {
    cleanup();
}

void GraphicsEngine::renderScene(Simulation& sim) {
    cam->update();
    if (cam->width != fbWidth || cam->height != fbHeight) {
        resizeHDRFramebuffer(cam->width, cam->height);
    }

    // Main scene objects pass

    glBindFramebuffer(GL_FRAMEBUFFER, hdrFBO);
    glViewport(0, 0, fbWidth, fbHeight);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glm::vec3 sunPos = sim.renderables["sun"]->pos;
    glm::vec3 earthPos = sim.renderables["earth"]->pos;
    for (auto& [id, rend] : sim.renderables) {
        rend->setSunPos(sunPos);
        rend->draw(*cam);
    }

    // Atmospheric scattering pass

    glBindFramebuffer(GL_FRAMEBUFFER, atmoFBO);
    glViewport(0, 0, fbWidth, fbHeight);
    glDisable(GL_DEPTH_TEST);
    glDepthMask(GL_FALSE);
    glEnable(GL_BLEND);
    glBlendFunc(GL_ONE, GL_ONE);

    Shader& atmo = shaders["atmosphere"];
    atmo.use();
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, hdrDepthTex);
    glm::vec3 planetCenterRelative = earthPos - cam->position;
    atmo.setVec3("uPlanetCenterRel", planetCenterRelative);
    atmo.setInt("uSceneDepth", 0);
    atmo.setMat4("uInvProj", glm::inverse(cam->projection));
    atmo.setMat4("uInvView", glm::inverse(cam->view));
    atmo.setFloat("uPlanetRadius", sim.renderables["earth"]->radius);
    atmo.setFloat("uAtmosRadius", sim.renderables["earth"]->radius * 3 * 1.0157f);
    atmo.setVec3("uSunDir", glm::normalize(sunPos - earthPos));
    // float K = toRender(1.0f);
    float K = 1e-2;
    atmo.setFloat("uRayleighScaleHeight", 8500.0f * K);
    atmo.setFloat("uMieScaleHeight", 1200.0f * K);
    atmo.setVec3("uRayleighCoeff", glm::vec3(5.5e-6f, 13.0e-6f, 22.4e-6f) / K);
    atmo.setFloat("uMieCoeff", 21e-6f / K);
    atmo.setInt("uNumSamples", 16);
    atmo.setInt("uNumLightSamples", 8);
    atmo.setFloat("uMieG", 0.758f);

    glBindVertexArray(quadVAO);
    glDrawArrays(GL_TRIANGLES, 0, 6);
    glBindVertexArray(0);
    glDisable(GL_BLEND);
    glDepthMask(GL_TRUE);
    glEnable(GL_DEPTH_TEST);

    // Tonemapping pass

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glDisable(GL_DEPTH_TEST);
    glClear(GL_COLOR_BUFFER_BIT);

    Shader& tm = shaders["tonemap"];
    tm.use();
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, hdrColorTex);
    tm.setInt("uHDRBuffer", 0);
    tm.setFloat("uExposure", 1.0f);

    glBindVertexArray(quadVAO);
    glDrawArrays(GL_TRIANGLES, 0, 6);
    glBindVertexArray(0);
    glEnable(GL_DEPTH_TEST);
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
    destroyHDRFramebuffer();
    glfwDestroyWindow(window);
    glfwTerminate();
}

void GraphicsEngine::createHDRFramebuffer(int width, int height) {
    fbWidth = width;
    fbHeight = height;

    glGenFramebuffers(1, &hdrFBO);
    glBindFramebuffer(GL_FRAMEBUFFER, hdrFBO);

    // Color: floating point so scattering can exceed 1.0 without clipping
    glGenTextures(1, &hdrColorTex);
    glBindTexture(GL_TEXTURE_2D, hdrColorTex);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, width, height, 0, GL_RGBA, GL_FLOAT, nullptr);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, hdrColorTex, 0);

    // Depth as a TEXTURE (not a renderbuffer) so the atmosphere pass can sample it
    glGenTextures(1, &hdrDepthTex);
    glBindTexture(GL_TEXTURE_2D, hdrDepthTex);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT32F, width, height, 0,
                 GL_DEPTH_COMPONENT, GL_FLOAT, nullptr);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, hdrDepthTex, 0);

    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
        fprintf(stderr, "HDR framebuffer incomplete\n");
    }
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    glGenFramebuffers(1, &atmoFBO);
    glBindFramebuffer(GL_FRAMEBUFFER, atmoFBO);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, hdrColorTex, 0);
    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
        fprintf(stderr, "Atmosphere framebuffer incomplete\n");
    }
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void GraphicsEngine::destroyHDRFramebuffer() {
    glDeleteTextures(1, &hdrColorTex);
    glDeleteTextures(1, &hdrDepthTex);
    glDeleteFramebuffers(1, &hdrFBO);
    glDeleteFramebuffers(1, &atmoFBO);
}

void GraphicsEngine::resizeHDRFramebuffer(int width, int height) {
    if (width == 0 || height == 0) return;
    if (width == fbWidth && height == fbHeight) return;
    destroyHDRFramebuffer();
    createHDRFramebuffer(width, height);
}

void GraphicsEngine::setupScreenQuad() {
    float quadVerts[] = {
        // pos      // uv
        -1.f,  1.f,  0.f, 1.f,
        -1.f, -1.f,  0.f, 0.f,
         1.f, -1.f,  1.f, 0.f,
        -1.f,  1.f,  0.f, 1.f,
         1.f, -1.f,  1.f, 0.f,
         1.f,  1.f,  1.f, 1.f,
    };
    glGenVertexArrays(1, &quadVAO);
    glGenBuffers(1, &quadVBO);
    glBindVertexArray(quadVAO);
    glBindBuffer(GL_ARRAY_BUFFER, quadVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(quadVerts), quadVerts, GL_STATIC_DRAW);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4*sizeof(float), (void*)0);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4*sizeof(float), (void*)(2*sizeof(float)));
    glEnableVertexAttribArray(0);
    glEnableVertexAttribArray(1);
    glBindVertexArray(0);
}

void GraphicsEngine::loadTexture(const std::string& key) {
    GLuint texture;
    glGenTextures(1, &texture);

    size_t delim = key.find("/");
    std::string tex_type = key.substr(0, delim);
    std::string tex_name = key.substr(delim+1);
    printf("Loading texture '%s/%s'\n", tex_type.c_str(), tex_name.c_str());

    if (tex_type == "uvmap") {
        glBindTexture(GL_TEXTURE_2D, texture);
        
        // set the texture wrapping/filtering options (on the currently bound texture object)
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);	
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        // load and generate the texture
        int width, height, nrChannels;
        std::string fp = std::format("resources/assets/{}/{}.png", tex_type, tex_name);
        uint8_t* data = stbi_load(fp.c_str(), &width, &height, &nrChannels, 0);
        if (data) {
            GLenum internalFormat = (nrChannels == 4) ? GL_SRGB8_ALPHA8 : GL_SRGB8;
            GLenum format = (nrChannels == 4) ? GL_RGBA : (nrChannels == 1 ? GL_RED : GL_RGB);
            glTexImage2D(
                GL_TEXTURE_2D, 0, internalFormat, width, height, 0, 
                format, GL_UNSIGNED_BYTE, data
            );
            glGenerateMipmap(GL_TEXTURE_2D);
        } else {
            printf("Failed to load uvmap: '%s'\n", fp.c_str());
        }
        stbi_image_free(data);
        textures.emplace(key, texture);
        printf("uvmap loaded: '%s' id=%d\n", key.c_str(), texture);

    } else if (tex_type == "cubemap") {
        glBindTexture(GL_TEXTURE_CUBE_MAP, texture);
        std::vector<std::string> faces{
            "right", "left", "top", "bottom", "front", "back"
        };

        glPixelStorei(GL_UNPACK_ALIGNMENT, 1); // avoid row padding assumptions

        int width, height, nrChannels;
        for (size_t i = 0; i < faces.size(); i++) {
            std::string fp = std::format("resources/assets/{}/{}/{}.png", tex_type, tex_name, faces[i]);
            uint8_t* data = stbi_load(fp.c_str(), &width, &height, &nrChannels, 0);
            if (data) {
                GLenum internalFormat = (nrChannels == 4) ? GL_SRGB8_ALPHA8 : GL_SRGB8;
                GLenum format = (nrChannels == 4) ? GL_RGBA : (nrChannels == 1 ? GL_RED : GL_RGB);
                glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 
                    0, internalFormat, width, height, 0, format, GL_UNSIGNED_BYTE, data
                );
            } else {
                printf("Failed to load cubemap: '%s'\n", fp.c_str());
            }
            stbi_image_free(data);
        }

        glPixelStorei(GL_UNPACK_ALIGNMENT, 4); // restore default for subsequent loads

        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
        textures.emplace(key, texture);
        printf("cubemap loaded: '%s' id=%d\n", key.c_str(), texture);
    }
}

GLuint& GraphicsEngine::getTextureID(const std::string& key) {
    return textures.at(key);
}

Shader& GraphicsEngine::getShader(const std::string& key) {
    return shaders.at(key);
}

void GraphicsEngine::handleError(int error, const char* description) {
    fprintf(stderr, "Graphics Engine Error %d: %s\n", error, description);
}

