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
    glm::vec3 baseColor;
    Shader shader;
    int textureID;
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
