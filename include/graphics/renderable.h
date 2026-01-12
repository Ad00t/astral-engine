#ifndef RENDERABLE_H
#define RENDERABLE_H

#include "glm/ext/vector_float3.hpp"
#include "opengl_includes.h"
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <vector>
#include <memory>
#include <cstdint>

struct Vertex {
    glm::vec3 pos;
    glm::vec3 normal;
    glm::vec2 uv;   
};

class GraphicsEngine;

class Renderable {
protected:
    std::weak_ptr<GraphicsEngine> gEng;
    GLuint VAO, VBO, EBO;
    glm::mat4 model;
    GLsizei indexCount;
    std::vector<Vertex> vertices;
    std::vector<uint32_t> indices;
    
    void setupMesh(const std::vector<Vertex>& vertices,
                   const std::vector<uint32_t>& indices);

public:
    Renderable(std::weak_ptr<GraphicsEngine> gEng);
    virtual ~Renderable();

    virtual void draw(const glm::mat4& view, const glm::mat4& projection);

    void setModel(const glm::mat4& model);
    glm::mat4& getModel();
};

class Cube : public Renderable {
private:
    glm::vec3 color;

public:
    Cube(std::weak_ptr<GraphicsEngine> gEng, glm::vec3 color);

    void draw(const glm::mat4& view, const glm::mat4& projection) override;
};

class Sphere : public Renderable {
private:
    float radius;
    glm::vec3 color;

public:
    Sphere(std::weak_ptr<GraphicsEngine> gEng, glm::vec3 color, double realRadius, 
           int sectorCount = 36, int stackCount = 18);
    
    void draw(const glm::mat4& view, const glm::mat4& projection) override;
};

#endif // RENDERABLE_H
