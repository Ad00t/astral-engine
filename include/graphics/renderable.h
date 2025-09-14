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
public:
    Renderable(const std::vector<float>& vertices,
               const std::vector<unsigned int>& indices,
               std::shared_ptr<Shader> shader,
               glm::vec3 color);
    virtual ~Renderable();

    virtual void draw(const glm::mat4& view, const glm::mat4& projection);

    void setModel(const glm::mat4& model);
    glm::mat4& getModel();

    std::shared_ptr<Shader> getShader();

protected:
    GLuint VAO, VBO, EBO;
    std::shared_ptr<Shader> shader;
    glm::mat4 model;
    glm::vec3 color;
    GLsizei indexCount;

    void setupMesh(const std::vector<float>& vertices,
                   const std::vector<unsigned int>& indices);
};

class Cube : public Renderable {
public:
    Cube(std::shared_ptr<Shader> shader, glm::vec3 color);
    Cube(std::shared_ptr<Shader> shader, glm::vec3 color, const glm::mat4& initialModel);
};

class Sphere : public Renderable {
public:
    Sphere(std::shared_ptr<Shader> shader, glm::vec3 color, float radius, 
           unsigned int sectorCount = 36, unsigned int stackCount = 18);
    Sphere(std::shared_ptr<Shader> shader, glm::vec3 color, float radius, const glm::mat4& initialModel,
           unsigned int sectorCount = 36, unsigned int stackCount = 18);
};

#endif // RENDERABLE_H
