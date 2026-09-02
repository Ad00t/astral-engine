#ifndef RENDERABLE_H
#define RENDERABLE_H

#include "glm/ext/vector_float3.hpp"
#include "opengl_includes.h"
#include "graphics/camera.h"
#include "graphics/shader.h"
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <vector>
#include <cstdint>

class GraphicsEngine;

struct Vertex {
    glm::vec3 pos;
    glm::vec3 normal;
    glm::vec2 uv;   
};

struct Material {
    Shader shader;
    int textureID;
    int textureID2;
   
    // entity uniforms
    glm::vec4 uBaseColor = glm::vec4(1.0f);
    int uTextureMap = 0;
    bool uUseTexture = true;
    int uNightTextureMap = 1;
    bool uUseDayNightBlend = false;
    glm::vec3 uSunPos = glm::vec3(0.0f);
    glm::vec3 uAmbientLighting = glm::vec3(0.1f);
    glm::vec3 uNightAmbientBoost = glm::vec3(1.0f);
    glm::vec3 uEmissiveLighting = glm::vec3(0.0f);
};

class Renderable {
protected:
    GLuint VAO, VBO, EBO;
    glm::mat4 model;
    GLsizei indexCount;
    Material mat;
    std::vector<Vertex> vertices;
    std::vector<uint32_t> indices;

    void setupMesh(const std::vector<Vertex>& vertices, const std::vector<uint32_t>& indices);
public:
    float radius;

    Renderable(Material mat, float radius);
    Renderable(const Renderable&) = delete;
    Renderable& operator=(const Renderable&) = delete;
    Renderable(Renderable&&) noexcept;
    Renderable& operator=(Renderable&&) noexcept;

    virtual ~Renderable();
    virtual void draw(const Camera& cam) = 0;

    void setModel(const glm::mat4& model);
    glm::mat4& getModel();
    void setSunPos(const glm::vec3& sunPos);
};

class SkyBox : public Renderable {
public:
    SkyBox(Material mat);
    void draw(const Camera& cam) override;
};

class Cube : public Renderable {
public:
    Cube(Material mat, float sideLength);
    void draw(const Camera& cam) override;
};

class Sphere : public Renderable {
public:
    Sphere(Material mat, float radius); 
    void draw(const Camera& cam) override;
};

#endif // RENDERABLE_H
