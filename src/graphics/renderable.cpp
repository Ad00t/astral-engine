#include "graphics/renderable.h"
#include "glad/gl.h"
#include "graphics/graphics_engine.h"
#include "utils.h"
#include "opengl_includes.h"
#include "graphics/shader.h"
#define STB_IMAGE_IMPLEMENTATION
#include "graphics/stb_image.h"
#include <vector>
#include <glm/gtc/type_ptr.hpp>
#include <memory>
#include <cmath>
#include <cstdint>

#ifndef offsetof
#define offsetof(t, d) __builtin_offsetof(t, d)
#endif

Renderable::Renderable(std::weak_ptr<GraphicsEngine> gEng, glm::vec3 color, std::string texture)
    : gEng(gEng), VAO(0), VBO(0), EBO(0), color(color), model(glm::mat4(1.0f)), indexCount(0), vertices(std::vector<Vertex>()), indices(std::vector<uint32_t>()) {
    textureID = -1;
    std::shared_ptr<GraphicsEngine> lgEng = gEng.lock();
    if (!lgEng) return;
    if (!texture.empty()) {
        textureID = lgEng->getTextureID(texture);
    }
}

Renderable::~Renderable() {
    glDeleteVertexArrays(1, &VAO);
    glDeleteBuffers(1, &VBO);
    glDeleteBuffers(1, &EBO);
}

void Renderable::draw(const glm::mat4& view, const glm::mat4& projection) {}

void Renderable::setupMesh(const std::vector<Vertex>& vertices, const std::vector<uint32_t>& indices) {
    indexCount = static_cast<GLsizei>(indices.size());

    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    glGenBuffers(1, &EBO);

    glBindVertexArray(VAO);

    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(Vertex),
                 vertices.data(), GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(uint32_t),
                 indices.data(), GL_STATIC_DRAW);
    
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, pos));
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, normal));
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, uv));
    glEnableVertexAttribArray(0);
    glEnableVertexAttribArray(1);
    glEnableVertexAttribArray(2);

    glBindVertexArray(0);
}

void Renderable::setModel(const glm::mat4& m) {
    model = m;
}

glm::mat4& Renderable::getModel() {
    return model;
}

// CUBE

static const std::vector<Vertex> cubeVertices = {
    { { -0.5f, -0.5f, -0.5f } },
    { {  0.5f, -0.5f, -0.5f } },
    { {  0.5f,  0.5f, -0.5f } },
    { { -0.5f,  0.5f, -0.5f } },
    { { -0.5f, -0.5f,  0.5f } },
    { {  0.5f, -0.5f,  0.5f } },
    { {  0.5f,  0.5f,  0.5f } },
    { { -0.5f,  0.5f,  0.5f } }
};

static const std::vector<uint32_t> cubeIndices = {
    0,1,2, 2,3,0,
    4,5,6, 6,7,4,
    0,4,7, 7,3,0,
    1,5,6, 6,2,1,
    3,2,6, 6,7,3,
    0,1,5, 5,4,0
};

Cube::Cube(std::weak_ptr<GraphicsEngine> gEng, glm::vec3 color, std::string texture)
    : Renderable(gEng, color, texture) {
    setupMesh(cubeVertices, cubeIndices);
}

void Cube::draw(const glm::mat4& view, const glm::mat4& projection) {
    std::shared_ptr<GraphicsEngine> lgEng = gEng.lock();
    if (!lgEng) return;
    Shader* simobjShader = lgEng->getShader("simobj");
    glUseProgram(simobjShader->ID);
    
    simobjShader->setMat4("model", model);
    simobjShader->setMat4("view", view);
    simobjShader->setMat4("projection", projection);
    simobjShader->setVec4("uMaterialColor", glm::vec4(color, 1.0)); 
    
    glBindVertexArray(VAO);
    glDrawElements(GL_TRIANGLES, indexCount, GL_UNSIGNED_INT, 0);
    glBindVertexArray(0);
}

// SPHERE

// helper to generate sphere vertices/indices
static void generateSphere(float radius, uint32_t sectorCount, uint32_t stackCount,
                            std::vector<Vertex>& vertices, std::vector<uint32_t>& indices) {
    float x, y, z, xy;
    float nx, ny, nz, lengthInv = 1.0f / radius;
    float u, v;

    for (uint32_t i = 0; i <= stackCount; ++i) {
        float stackAngle = M_PI / 2 - i * M_PI / stackCount;
        xy = radius * cosf(stackAngle);
        z  = radius * sinf(stackAngle);

        for (uint32_t j = 0; j <= sectorCount; ++j) {
            float sectorAngle = j * 2 * M_PI / sectorCount;
            x = xy * cosf(sectorAngle);
            y = xy * sinf(sectorAngle);

            // normal = position on a sphere centered at origin, normalized
            nx = x * lengthInv;
            ny = y * lengthInv;
            nz = z * lengthInv;

            // uv
            u = (float)j / sectorCount;
            v = (float)i / stackCount;

            vertices.push_back({ glm::vec3(x, y, z), glm::vec3(nx, ny, nz), glm::vec2(u, v) });
        }
    }

    // indices
    for (int i = 0; i < stackCount; ++i) {
        int k1 = i * (sectorCount + 1); // beginning of stack
        int k2 = k1 + sectorCount + 1;  // beginning of next stack

        for (int j = 0; j < sectorCount; ++j, ++k1, ++k2) {
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

Sphere::Sphere(std::weak_ptr<GraphicsEngine> gEng, glm::vec3 color, std::string texture, double realRadius, int sectorCount, int stackCount)
    : Renderable(gEng, color, texture), radius(toRender(realRadius)) {
    std::vector<Vertex> vertices;
    std::vector<uint32_t> indices;
    generateSphere(radius, sectorCount, stackCount, vertices, indices);
    setupMesh(vertices, indices);
}

void Sphere::draw(const glm::mat4& view, const glm::mat4& projection) {
    std::shared_ptr<GraphicsEngine> lgEng = gEng.lock();
    if (!lgEng) return;
    Shader* simobjShader = lgEng->getShader("simobj");
    simobjShader->use();

    simobjShader->setMat4("model", model);
    simobjShader->setMat4("view", view);
    simobjShader->setMat4("projection", projection);
    simobjShader->setVec4("uMaterialColor", glm::vec4(color, 1.0)); 

    simobjShader->setBool("uUseTexture", textureID != -1);
    simobjShader->setInt("uTextureMap", 0);   
    if (textureID != -1) {
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, textureID);
    }

    glBindVertexArray(VAO);
    glDrawElements(GL_TRIANGLES, indexCount, GL_UNSIGNED_INT, 0);
    glBindVertexArray(0);
}
