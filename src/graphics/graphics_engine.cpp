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
#include <ranges>
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
    shaders["entity"] = Shader("entity.vert", "entity.frag");
    shaders["skybox"] = Shader("skybox.vert", "skybox.frag");

    // Load textures
    // stbi_set_flip_vertically_on_load(true);
    loadTexture("uvmap/earth_day");
    loadTexture("uvmap/earth_night");
    loadTexture("uvmap/sun");
    loadTexture("uvmap/moon");
    loadTexture("cubemap/spacebox");

    int fbWidth, fbHeight;
    glfwGetFramebufferSize(window, &fbWidth, &fbHeight);
    glViewport(0, 0, fbWidth, fbHeight);
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_TEXTURE_CUBE_MAP_SEAMLESS);

#if defined(_WIN32)
    timeBeginPeriod(1); // Sets the minimum OS sleep granularity to 1ms
#endif

    cam = std::make_unique<Camera>(window, 5e7f, 1e6f, 1e22f, 0.01f, 0.01f, 10.0f);
    printf("Graphics engine initialized\n");
}

GraphicsEngine::~GraphicsEngine() {
    cleanup();
}

void GraphicsEngine::renderScene(std::unordered_map<std::string, std::unique_ptr<Renderable>>& renderables) {
    cam->update();
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glm::vec3 sunPos = glm::vec3(renderables["sun"]->getModel()[3]);
    for (auto& [id, rend] : renderables) {
        rend->setSunPos(sunPos);
        rend->draw(*cam);
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

void GraphicsEngine::loadTexture(const std::string& key) {
    uint32_t texture;
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
            GLenum format = (nrChannels == 4) ? GL_RGBA : (nrChannels == 1 ? GL_RED : GL_RGB);
            glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
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
                GLenum format = (nrChannels == 4) ? GL_RGBA : (nrChannels == 1 ? GL_RED : GL_RGB);
                glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 
                    0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data
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

int& GraphicsEngine::getTextureID(const std::string& key) {
    return textures.at(key);
}

Shader& GraphicsEngine::getShader(const std::string& key) {
    return shaders.at(key);
}

void GraphicsEngine::handleError(int error, const char* description) {
    fprintf(stderr, "Graphics Engine Error %d: %s\n", error, description);
}

