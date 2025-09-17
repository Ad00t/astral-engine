#ifndef RENDERABLE_H
#define RENDERABLE_H

#include "glm/ext/vector_float3.hpp"
#include "opengl_includes.h"
#include "graphics/shader.h"
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <vector>
#include <memory>

class Renderable {
protected:
    GLuint VAO, VBO, EBO;
    Shader* shader;
    glm::mat4 model;
    glm::vec3 color;
    GLsizei indexCount;

    void setupMesh(const std::vector<float>& vertices,
                   const std::vector<unsigned int>& indices);

public:
    Renderable(const std::vector<float>& vertices,
               const std::vector<unsigned int>& indices,
               Shader* shader,
               glm::vec3 color);
    virtual ~Renderable();

    virtual void draw(const glm::mat4& view, const glm::mat4& projection);

    void setModel(const glm::mat4& model);
    glm::mat4& getModel();

    Shader* getShader();
};

class Cube : public Renderable {
public:
    Cube(Shader* shader, glm::vec3 color);
    Cube(Shader* shader, glm::vec3 color, const glm::mat4& initialModel);
};

class Sphere : public Renderable {
private:
    float radius;

public:
    Sphere(Shader* shader, glm::vec3 color, double realRadius, 
           unsigned int sectorCount = 36, unsigned int stackCount = 18);
    Sphere(Shader* shader, glm::vec3 color, double realRadius, const glm::mat4& initialModel,
           unsigned int sectorCount = 36, unsigned int stackCount = 18);
};

#endif // RENDERABLE_H
