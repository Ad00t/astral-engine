#include "graphics/graphics_engine.h"
#include "glad/gl.h"
#include "graphics/shader.h"
#include "graphics/camera.h"
#include "graphics/renderable.h"
#include "core/update_limiter.h"
#include "graphics/stb_image.h"
#include "utils.h"
#include <GLFW/glfw3.h>
#include <filesystem>
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
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 5);
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
    glClipControl(GL_LOWER_LEFT, GL_ZERO_TO_ONE);

#ifndef __APPLE__
    glEnable(GL_DEBUG_OUTPUT);
    glDebugMessageCallback([](GLenum, GLenum, GLuint, GLenum severity, GLsizei, const GLchar* msg, const void*) {
        if (severity != GL_DEBUG_SEVERITY_NOTIFICATION)
            fprintf(stderr, "GL: %s\n", msg);
    }, nullptr);
#endif
   
    // Load shaders
    shaders.emplace("entity", Shader("entity.vert", "entity.frag"));
    shaders.emplace("skybox", Shader("skybox.vert", "skybox.frag"));
    shaders.emplace("atmosphere", Shader("fullscreen.vert", "atmosphere.frag"));
    shaders.emplace("bloom", Shader("fullscreen.vert", "bloom.frag"));
    shaders.emplace("tonemap", Shader("fullscreen.vert", "tonemap.frag"));

    // Load textures
    // stbi_set_flip_vertically_on_load(true);
    textures.emplace("uvmap/earth_day", loadTextureFromFile("resources/assets/uvmaps/earth_day.png"));
    textures.emplace("uvmap/earth_night", loadTextureFromFile("resources/assets/uvmaps/earth_night.png"));
    textures.emplace("uvmap/sun", loadTextureFromFile("resources/assets/uvmaps/sun.png"));
    textures.emplace("uvmap/moon", loadTextureFromFile("resources/assets/uvmaps/moon.png"));
    textures.emplace("cubemap/spacebox", loadTextureCubemap("resources/assets/cubemaps/spacebox"));

    glfwGetFramebufferSize(window, &width, &height);
    glViewport(0, 0, width, height);
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    glEnable(GL_DEPTH_TEST);
    glClearDepth(0.0f);       // "far" is now 0, not 1
    glDepthFunc(GL_GREATER);  // closer geometry now has a LARGER depth value, not smaller
    glEnable(GL_TEXTURE_CUBE_MAP_SEAMLESS);

#if defined(_WIN32)
    timeBeginPeriod(1); // Sets the minimum OS sleep granularity to 1ms
#endif

    cam = std::make_unique<Camera>(window, 5e7f, 1e6f, 1e22f, 0.01f, 0.01f, 0.1f);

    createRenderTargets(width, height);
    setupScreenQuad();

    printf("Graphics engine initialized\n");
}

GraphicsEngine::~GraphicsEngine() {
    cleanup();
}

void GraphicsEngine::renderScene(Simulation& sim) {
    cam->update();
    if (cam->width != width || cam->height != height) {
        resizeRenderTargets(cam->width, cam->height);
    }

    // Geometry pass

    glBindFramebuffer(GL_FRAMEBUFFER, hdrFBO);
    glViewport(0, 0, width, height);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    for (auto& [id, rend] : sim.renderables) {
        glm::vec3 relPos = toRenderUnits(rend->realPos - cam->realPos);
        glm::mat4 model = glm::translate(glm::mat4(1.0f), relPos) * rend->rotation;
        model = glm::scale(model, glm::vec3(rend->renderScale));
        rend->setModel(model);
        rend->renderPos = relPos;
    }

    for (auto& [id, rend] : sim.renderables) {
        if (id == "spacebox") continue;
        rend->setSunRenderPos(sim.renderables["sun"]->renderPos);
        rend->draw(*cam);
    }
    sim.renderables["spacebox"]->draw(*cam);

    // glBindFramebuffer(GL_READ_FRAMEBUFFER, hdrFBO);
    // glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
    // glBlitFramebuffer(
    //     0, 0, width, height,
    //     0, 0, width, height,
    //     GL_COLOR_BUFFER_BIT,
    //     GL_NEAREST
    // );
    // return;

    // Atmospheric scattering pass

    glBindFramebuffer(GL_FRAMEBUFFER, atmoFBO);
    glViewport(0, 0, width, height);
    glDisable(GL_DEPTH_TEST);
    glDepthMask(GL_FALSE);
    glEnable(GL_BLEND);
    glBlendFunc(GL_ONE, GL_ONE);

    Shader& atmo = shaders["atmosphere"];
    atmo.use();
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, hdrDepthTex);
    atmo.setInt("uSceneDepth", 0);
    atmo.setMat4("uInvProj", glm::inverse(cam->projection));
    atmo.setMat4("uInvView", glm::inverse(cam->view));

    glBindVertexArray(quadVAO);
    for (auto& [id, rend] : sim.renderables) {
        const AtmosphereParams& atmos = rend->getMaterial().atmosphere;
        if (!atmos.enabled) continue;
        atmo.setVec3("uPlanetPosRel", toRenderUnits(rend->realPos - cam->realPos));
        atmo.setFloat("uPlanetRadius", rend->renderScale);
        atmo.setFloat("uAtmosRadius", rend->renderScale * atmos.radiusMultiplier);
        atmo.setVec3("uSunDir", glm::normalize(glm::vec3(sim.renderables["sun"]->realPos - rend->realPos)));
        atmo.setFloat("uRayleighScaleHeight", atmos.rayleighScaleHeight);
        atmo.setFloat("uMieScaleHeight", atmos.mieScaleHeight);
        atmo.setVec3("uRayleighCoeff", atmos.rayleighCoeff);
        atmo.setFloat("uMieCoeff", atmos.mieCoeff);
        atmo.setInt("uNumSamples", atmos.numSamples);
        atmo.setInt("uNumLightSamples", atmos.numLightSamples);
        atmo.setFloat("uMieG", atmos.mieG);
        glDrawArrays(GL_TRIANGLES, 0, 6);
    }
    glBindVertexArray(0);

    glDisable(GL_BLEND);
    glDepthMask(GL_TRUE);
    glEnable(GL_DEPTH_TEST);

    // Bloom pass

    bool horizontal = true;
    Shader& bloomShader = shaders["bloom"];
    bloomShader.use();
    glViewport(0, 0, bloomWidth, bloomHeight);
    for (int i = 0; i < NUM_BLOOM_PASSES; i++) {
        glBindFramebuffer(GL_FRAMEBUFFER, bloomFBO[horizontal]); 
        bloomShader.setBool("uHorizontal", horizontal);
        glBindTexture(
            GL_TEXTURE_2D, i == 0 ? hdrColorTex[1] : bloomColorTex[!horizontal]
        ); 
        glBindVertexArray(quadVAO);
        glDrawArrays(GL_TRIANGLES, 0, 6);
        glBindVertexArray(0);
        horizontal = !horizontal;
    }

    // Composite + tonemapping pass

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glDisable(GL_DEPTH_TEST);
    glClear(GL_COLOR_BUFFER_BIT);

    Shader& tonemapShader = shaders["tonemap"];
    tonemapShader.use();
    glViewport(0, 0, width, height);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, hdrColorTex[0]);
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, bloomColorTex[NUM_BLOOM_PASSES % 2]);
    tonemapShader.setInt("uHDRColorTex", 0);
    tonemapShader.setInt("uBloomColorTex", 1);
    tonemapShader.setFloat("uExposure", 1.0f);

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
    destroyRenderTargets();
    glfwDestroyWindow(window);
    glfwTerminate();
}

void GraphicsEngine::createRenderTargets(int w, int h) {
    width = w; 
    height = h;
    bloomWidth = width / BLOOM_DOWNSAMPLE;
    bloomHeight = height / BLOOM_DOWNSAMPLE;

    glGenFramebuffers(1, &hdrFBO);
    glBindFramebuffer(GL_FRAMEBUFFER, hdrFBO);

    // Color: 1 for FragColor 2 for BrightColor

    glGenTextures(2, hdrColorTex);
    for (int i = 0; i < 2; i++) {
        glBindTexture(GL_TEXTURE_2D, hdrColorTex[i]);
        glTexImage2D(
            GL_TEXTURE_2D, 0, GL_RGBA16F, width, height, 0, GL_RGBA, GL_FLOAT, NULL
        );
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        // attach texture to framebuffer
        glFramebufferTexture2D(
            GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + i, GL_TEXTURE_2D, hdrColorTex[i], 0
        );
    }  
    GLuint attachments[2] = { GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1 };
    glDrawBuffers(2, attachments);  

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

    // Atmosphere

    glGenFramebuffers(1, &atmoFBO);
    glBindFramebuffer(GL_FRAMEBUFFER, atmoFBO);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, hdrColorTex[0], 0);
    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
        fprintf(stderr, "Atmosphere framebuffer incomplete\n");
    }
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    // Bloom

    glGenFramebuffers(2, bloomFBO);
    glGenTextures(2, bloomColorTex);
    for (int i = 0; i < 2; i++) {
        glBindFramebuffer(GL_FRAMEBUFFER, bloomFBO[i]);
        glBindTexture(GL_TEXTURE_2D, bloomColorTex[i]);
        glTexImage2D(
            GL_TEXTURE_2D, 0, GL_RGBA16F, bloomWidth, bloomHeight, 0, GL_RGBA, GL_FLOAT, NULL
        );
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glFramebufferTexture2D(
            GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, bloomColorTex[i], 0
        );
    }

}

void GraphicsEngine::destroyRenderTargets() {
    glDeleteFramebuffers(1, &hdrFBO);
    glDeleteFramebuffers(1, &atmoFBO);
    glDeleteFramebuffers(2, bloomFBO);
    glDeleteTextures(2, hdrColorTex);
    glDeleteTextures(1, &hdrDepthTex);
    glDeleteTextures(2, bloomColorTex);
}

void GraphicsEngine::resizeRenderTargets(int w, int h) {
    if (width == 0 || height == 0) return;
    if (width == w && height == h) return;
    destroyRenderTargets();
    createRenderTargets(w, h);
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

static GLuint uploadRGBAOrRGB(uint8_t* data, int width, int height, int nrChannels) {
    GLuint texture;
    glGenTextures(1, &texture);
    glBindTexture(GL_TEXTURE_2D, texture);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    GLenum internalFormat = (nrChannels == 4) ? GL_SRGB8_ALPHA8 : GL_SRGB8;
    GLenum format = (nrChannels == 4) ? GL_RGBA : (nrChannels == 1 ? GL_RED : GL_RGB);
    glTexImage2D(GL_TEXTURE_2D, 0, internalFormat, width, height, 0, format, GL_UNSIGNED_BYTE, data);
    glGenerateMipmap(GL_TEXTURE_2D);
    return texture;
}

GLuint GraphicsEngine::loadTextureCubemap(const std::string& path) {
    GLuint texture;
    glGenTextures(1, &texture);
    glBindTexture(GL_TEXTURE_CUBE_MAP, texture);

    static const std::vector<std::string> faces{
        "right", "left", "top", "bottom", "front", "back"
    };

    glPixelStorei(GL_UNPACK_ALIGNMENT, 1); // avoid row padding assumptions

    int width, height, nrChannels;
    for (size_t i = 0; i < faces.size(); i++) {
        std::string file = std::format("{}.png", faces[i]);
        std::filesystem::path fp = path;
        fp /= file;
        uint8_t* data = stbi_load(fp.string().c_str(), &width, &height, &nrChannels, 0);
        if (data) {
            GLenum internalFormat = (nrChannels == 4) ? GL_SRGB8_ALPHA8 : GL_SRGB8;
            GLenum format = (nrChannels == 4) ? GL_RGBA : (nrChannels == 1 ? GL_RED : GL_RGB);
            glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i,
                0, internalFormat, width, height, 0, format, GL_UNSIGNED_BYTE, data);
        } else {
            printf("Failed to load cubemap face: '%s'\n", fp.string().c_str());
        }
        stbi_image_free(data);
    }

    glPixelStorei(GL_UNPACK_ALIGNMENT, 4); // restore default

    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

    return texture;
}

GLuint GraphicsEngine::loadTextureFromFile(const std::string& path) {
    int width, height, nrChannels;
    uint8_t* data = stbi_load(path.c_str(), &width, &height, &nrChannels, 0);
    if (!data) {
        printf("Failed to load texture: '%s'\n", path.c_str());
        return 0;
    }
    GLuint texture = uploadRGBAOrRGB(data, width, height, nrChannels);
    stbi_image_free(data);
    return texture;
}

GLuint GraphicsEngine::loadTextureFromMemory(const uint8_t* bytes, size_t size) {
    int width, height, nrChannels;
    uint8_t* data = stbi_load_from_memory(bytes, static_cast<int>(size), &width, &height, &nrChannels, 0);
    if (!data) {
        printf("Failed to decode embedded texture (%zu bytes)\n", size);
        return 0;
    }
    GLuint texture = uploadRGBAOrRGB(data, width, height, nrChannels);
    stbi_image_free(data);
    return texture;
}

GLuint GraphicsEngine::loadTextureFromRawRGBA(const uint8_t* data, int width, int height) {
    return uploadRGBAOrRGB(const_cast<uint8_t*>(data), width, height, 4);
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

