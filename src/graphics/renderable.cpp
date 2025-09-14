#include "graphics/renderable.h"
#include "opengl_includes.h"
#include "graphics/shader.h"
#include <vector>
#include <glm/gtc/type_ptr.hpp>
#include <memory>

Renderable::Renderable(const std::vector<float>& vertices,
                       const std::vector<unsigned int>& indices,
                       std::shared_ptr<Shader> shader,
                       glm::vec3 color)
    : VAO(0), VBO(0), EBO(0), shader(shader), color(color), model(glm::mat4(1.0f)) {
    setupMesh(vertices, indices);
}

Renderable::~Renderable() {
    glDeleteVertexArrays(1, &VAO);
    glDeleteBuffers(1, &VBO);
    glDeleteBuffers(1, &EBO);
}

void Renderable::setupMesh(const std::vector<float>& vertices,
                           const std::vector<unsigned int>& indices) {
    indexCount = static_cast<GLsizei>(indices.size());

    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    glGenBuffers(1, &EBO);

    glBindVertexArray(VAO);

    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(float),
                 vertices.data(), GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned int),
                 indices.data(), GL_STATIC_DRAW);

    // assume positions only, 3 floats per vertex
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    glBindVertexArray(0);
}

void Renderable::draw(const glm::mat4& view, const glm::mat4& projection) {
    GLint modelLoc = glGetUniformLocation(shader->ID, "model");
    GLint viewLoc  = glGetUniformLocation(shader->ID, "view");
    GLint projLoc  = glGetUniformLocation(shader->ID, "projection");
    GLint colorLoc = glGetUniformLocation(shader->ID, "objectColor"); 

    glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
    glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
    glUniformMatrix4fv(projLoc, 1, GL_FALSE, glm::value_ptr(projection));
    glUniform3f(colorLoc, color[0], color[1], color[2]);
    
    glBindVertexArray(VAO);
    glDrawElements(GL_TRIANGLES, indexCount, GL_UNSIGNED_INT, 0);
    glBindVertexArray(0);
}

void Renderable::setModel(const glm::mat4& m) {
    model = m;
}

glm::mat4& Renderable::getModel() {
    return model;
}

std::shared_ptr<Shader> Renderable::getShader() {
    return shader;
}

// CUBE

static const std::vector<float> cubeVertices = {
   -0.5f, -0.5f, -0.5f,
    0.5f, -0.5f, -0.5f,
    0.5f,  0.5f, -0.5f,
   -0.5f,  0.5f, -0.5f,
   -0.5f, -0.5f,  0.5f,
    0.5f, -0.5f,  0.5f,
    0.5f,  0.5f,  0.5f,
   -0.5f,  0.5f,  0.5f
};

static const std::vector<unsigned int> cubeIndices = {
    0,1,2, 2,3,0,
    4,5,6, 6,7,4,
    0,4,7, 7,3,0,
    1,5,6, 6,2,1,
    3,2,6, 6,7,3,
    0,1,5, 5,4,0
};

Cube::Cube(std::shared_ptr<Shader> shader, glm::vec3 color)
: Renderable(cubeVertices, cubeIndices, shader, color) {}

Cube::Cube(std::shared_ptr<Shader> shader, glm::vec3 color, const glm::mat4& initialModel)
: Renderable(cubeVertices, cubeIndices, shader, color) {
    model = initialModel;
}

// SPHERE

// helper to generate sphere vertices/indices
static void generateSphere(float radius, unsigned int sectorCount, unsigned int stackCount,
                           std::vector<float>& vertices, std::vector<unsigned int>& indices) {
    const float PI = 3.14159265359f;
    float x, y, z, xy;                              // vertex position

    for (unsigned int i = 0; i <= stackCount; ++i) {
        float stackAngle = PI / 2 - i * PI / stackCount; // from pi/2 to -pi/2
        xy = radius * cosf(stackAngle);
        z = radius * sinf(stackAngle);

        for (unsigned int j = 0; j <= sectorCount; ++j) {
            float sectorAngle = j * 2 * PI / sectorCount; // 0 to 2pi
            x = xy * cosf(sectorAngle);
            y = xy * sinf(sectorAngle);
            vertices.push_back(x);
            vertices.push_back(y);
            vertices.push_back(z);
        }
    }

    // indices
    for (unsigned int i = 0; i < stackCount; ++i) {
        unsigned int k1 = i * (sectorCount + 1); // beginning of stack
        unsigned int k2 = k1 + sectorCount + 1;  // beginning of next stack

        for (unsigned int j = 0; j < sectorCount; ++j, ++k1, ++k2) {
            if (i != 0) {
                indices.push_back(k1);
                indices.push_back(k2);
                indices.push_back(k1 + 1);
            }

            if (i != (stackCount - 1)) {
                indices.push_back(k1 + 1);
                indices.push_back(k2);
                indices.push_back(k2 + 1);
            }
        }
    }
}

Sphere::Sphere(std::shared_ptr<Shader> shader, glm::vec3 color, float radius, unsigned int sectorCount, unsigned int stackCount)
    : Renderable(std::vector<float>(), std::vector<unsigned int>(), shader, color) {

    std::vector<float> vertices;
    std::vector<unsigned int> indices;
    generateSphere(radius, sectorCount, stackCount, vertices, indices);
    setupMesh(vertices, indices);
}

Sphere::Sphere(std::shared_ptr<Shader> shader, glm::vec3 color, float radius, const glm::mat4& initialModel,
               unsigned int sectorCount, unsigned int stackCount)
    : Renderable(std::vector<float>(), std::vector<unsigned int>(), shader, color) {

    std::vector<float> vertices;
    std::vector<unsigned int> indices;
    generateSphere(radius, sectorCount, stackCount, vertices, indices);
    setupMesh(vertices, indices);
    model = initialModel;
}
