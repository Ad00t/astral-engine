#include "graphics/renderable.h"
#include "glad/gl.h"
#include "graphics/graphics_engine.h"
#include "graphics/camera.h"
#include "utils.h"
#include "opengl_includes.h"
#include "graphics/shader.h"
#define STB_IMAGE_IMPLEMENTATION
#include "graphics/stb_image.h"
#include <vector>
#include <glm/gtc/type_ptr.hpp>
#include <cmath>
#include <cstdint>

#ifndef offsetof
#define offsetof(t, d) __builtin_offsetof(t, d)
#endif

Renderable::Renderable(Material mat, float renderScale)
    : VAO(0), VBO(0), EBO(0), mat(mat), realPos(glm::dvec3(0.0)), renderPos(glm::vec3(0.0f)), renderScale(renderScale), 
      model(glm::mat4(1.0f)), indexCount(0), vertices(std::vector<Vertex>()), indices(std::vector<uint32_t>()) {
}

Renderable::~Renderable() {
    glDeleteVertexArrays(1, &VAO);
    glDeleteBuffers(1, &VBO);
    glDeleteBuffers(1, &EBO);
}

void Renderable::draw(const Camera& cam) {}

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

void Renderable::bindMaterial(const Camera& cam) {
    mat.shader.use();
    mat.shader.setMat4("model", model);
    mat.shader.setMat4("view", cam.view);
    mat.shader.setMat4("projection", cam.projection);
    mat.shader.setVec4("uBaseColor", mat.uBaseColor);
    mat.shader.setInt("uTextureMap", mat.uTextureMap);
    mat.shader.setBool("uUseTexture", mat.uUseTexture);
    mat.shader.setInt("uNightTextureMap", mat.uNightTextureMap);
    mat.shader.setBool("uUseDayNightBlend", mat.uUseDayNightBlend);
    mat.shader.setVec3("uSunPos", mat.uSunPos);
    mat.shader.setVec3("uAmbientLighting", mat.uAmbientLighting);
    mat.shader.setVec3("uNightAmbientBoost", mat.uNightAmbientBoost);
    mat.shader.setVec3("uEmissiveLighting", mat.uEmissiveLighting);

    if (mat.uUseTexture) {
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, mat.textureID);
    }
    if (mat.uUseDayNightBlend) {
        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_2D, mat.textureID2);
    }
}

void Renderable::setModel(const glm::mat4& m) {
    model = m;
}

glm::mat4& Renderable::getModel() {
    return model;
}

void Renderable::setSunRenderPos(const glm::vec3& sunRenderPos) {
    mat.uSunPos = sunRenderPos;
}

// SKYBOX

static const std::vector<Vertex> skyboxVertices = {
    { { -1.0f,  1.0f, -1.0f } },
    { { -1.0f, -1.0f, -1.0f } },
    { { 1.0f, -1.0f, -1.0f } },
    { { 1.0f, -1.0f, -1.0f } },
    { { 1.0f,  1.0f, -1.0f } },
    { { -1.0f,  1.0f, -1.0f } },

    { { -1.0f, -1.0f,  1.0f } },
    { { -1.0f, -1.0f, -1.0f } },
    { { -1.0f,  1.0f, -1.0f } },
    { { -1.0f,  1.0f, -1.0f } },
    { { -1.0f,  1.0f,  1.0f } },
    { { -1.0f, -1.0f,  1.0f } },

    { { 1.0f, -1.0f, -1.0f } },
    { { 1.0f, -1.0f,  1.0f } },
    { { 1.0f,  1.0f,  1.0f } },
    { { 1.0f,  1.0f,  1.0f } },
    { { 1.0f,  1.0f, -1.0f } },
    { { 1.0f, -1.0f, -1.0f } },

    { { -1.0f, -1.0f,  1.0f } },
    { { -1.0f,  1.0f,  1.0f } },
    { { 1.0f,  1.0f,  1.0f } },
    { { 1.0f,  1.0f,  1.0f } },
    { { 1.0f, -1.0f,  1.0f } },
    { { -1.0f, -1.0f,  1.0f } },

    { { -1.0f,  1.0f, -1.0f } },
    { { 1.0f,  1.0f, -1.0f } },
    { { 1.0f,  1.0f,  1.0f } },
    { { 1.0f,  1.0f,  1.0f } },
    { { -1.0f,  1.0f,  1.0f } },
    { { -1.0f,  1.0f, -1.0f } },

    { { -1.0f, -1.0f, -1.0f } },
    { { -1.0f, -1.0f,  1.0f } },
    { { 1.0f, -1.0f, -1.0f } },
    { { 1.0f, -1.0f, -1.0f } },
    { { -1.0f, -1.0f,  1.0f } },
    { { 1.0f, -1.0f,  1.0f } }
};

SkyBox::SkyBox(Material mat)
    : Renderable(mat, 0) {
    setupMesh(skyboxVertices, std::vector<uint32_t>()); 
}

void SkyBox::draw(const Camera& cam) {
    glm::mat4 viewNoTranslation = glm::mat4(glm::mat3(cam.view));

    glDepthFunc(GL_GEQUAL);
    glDepthMask(GL_FALSE);

    mat.shader.use();
    mat.shader.setMat4("view", viewNoTranslation);
    mat.shader.setMat4("projection", cam.projection);

    glBindVertexArray(VAO);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_CUBE_MAP, mat.textureID);

    glDrawArrays(GL_TRIANGLES, 0, 36);

    glBindVertexArray(0);

    glDepthMask(GL_TRUE);
    glDepthFunc(GL_GREATER); 
}

// CUBE

static const std::vector<Vertex> cubeVertices = {
    // Front (+Z)
    { {-0.5f, -0.5f,  0.5f}, {0.0f, 0.0f, 1.0f}, {0.0f, 0.0f} },
    { { 0.5f, -0.5f,  0.5f}, {0.0f, 0.0f, 1.0f}, {1.0f, 0.0f} },
    { { 0.5f,  0.5f,  0.5f}, {0.0f, 0.0f, 1.0f}, {1.0f, 1.0f} },
    { {-0.5f,  0.5f,  0.5f}, {0.0f, 0.0f, 1.0f}, {0.0f, 1.0f} },

    // Back (-Z)
    { { 0.5f, -0.5f, -0.5f}, {0.0f, 0.0f, -1.0f}, {0.0f, 0.0f} },
    { {-0.5f, -0.5f, -0.5f}, {0.0f, 0.0f, -1.0f}, {1.0f, 0.0f} },
    { {-0.5f,  0.5f, -0.5f}, {0.0f, 0.0f, -1.0f}, {1.0f, 1.0f} },
    { { 0.5f,  0.5f, -0.5f}, {0.0f, 0.0f, -1.0f}, {0.0f, 1.0f} },

    // Left (-X)
    { {-0.5f, -0.5f, -0.5f}, {-1.0f, 0.0f, 0.0f}, {0.0f, 0.0f} },
    { {-0.5f, -0.5f,  0.5f}, {-1.0f, 0.0f, 0.0f}, {1.0f, 0.0f} },
    { {-0.5f,  0.5f,  0.5f}, {-1.0f, 0.0f, 0.0f}, {1.0f, 1.0f} },
    { {-0.5f,  0.5f, -0.5f}, {-1.0f, 0.0f, 0.0f}, {0.0f, 1.0f} },

    // Right (+X)
    { { 0.5f, -0.5f,  0.5f}, {1.0f, 0.0f, 0.0f}, {0.0f, 0.0f} },
    { { 0.5f, -0.5f, -0.5f}, {1.0f, 0.0f, 0.0f}, {1.0f, 0.0f} },
    { { 0.5f,  0.5f, -0.5f}, {1.0f, 0.0f, 0.0f}, {1.0f, 1.0f} },
    { { 0.5f,  0.5f,  0.5f}, {1.0f, 0.0f, 0.0f}, {0.0f, 1.0f} },

    // Top (+Y)
    { {-0.5f,  0.5f,  0.5f}, {0.0f, 1.0f, 0.0f}, {0.0f, 0.0f} },
    { { 0.5f,  0.5f,  0.5f}, {0.0f, 1.0f, 0.0f}, {1.0f, 0.0f} },
    { { 0.5f,  0.5f, -0.5f}, {0.0f, 1.0f, 0.0f}, {1.0f, 1.0f} },
    { {-0.5f,  0.5f, -0.5f}, {0.0f, 1.0f, 0.0f}, {0.0f, 1.0f} },

    // Bottom (-Y)
    { {-0.5f, -0.5f, -0.5f}, {0.0f, -1.0f, 0.0f}, {0.0f, 0.0f} },
    { { 0.5f, -0.5f, -0.5f}, {0.0f, -1.0f, 0.0f}, {1.0f, 0.0f} },
    { { 0.5f, -0.5f,  0.5f}, {0.0f, -1.0f, 0.0f}, {1.0f, 1.0f} },
    { {-0.5f, -0.5f,  0.5f}, {0.0f, -1.0f, 0.0f}, {0.0f, 1.0f} },
};

static const std::vector<uint32_t> cubeIndices = {
    0,  1,  2,  2,  3,  0,   // front
    4,  5,  6,  6,  7,  4,   // back
    8,  9,  10, 10, 11, 8,   // left
    12, 13, 14, 14, 15, 12,  // right
    16, 17, 18, 18, 19, 16,  // top
    20, 21, 22, 22, 23, 20,  // bottom
};

Cube::Cube(Material mat, float sideLength)
    : Renderable(mat, sideLength) {
    setupMesh(cubeVertices, cubeIndices);
}

void Cube::draw(const Camera& cam) {
    bindMaterial(cam);
    glBindVertexArray(VAO);
    glDrawElements(GL_TRIANGLES, indexCount, GL_UNSIGNED_INT, 0);
    glBindVertexArray(0);
}

// SPHERE

// helper to generate sphere vertices/indices
static void generateSphere(uint32_t sectorCount, uint32_t stackCount, std::vector<Vertex>& vertices, std::vector<uint32_t>& indices) {
    const float radius = 1.0f;

    for (uint32_t i = 0; i <= stackCount; ++i) {
        float stackAngle = M_PI / 2.0f - i * M_PI / stackCount;
        float xy = cosf(stackAngle);
        float z = sinf(stackAngle);

        for (uint32_t j = 0; j <= sectorCount; ++j) {
            float sectorAngle = j * 2.0f * M_PI / sectorCount;

            float x = xy * cosf(sectorAngle);
            float y = xy * sinf(sectorAngle);

            glm::vec3 position(x, y, z);
            glm::vec3 normal = glm::normalize(position);

            float u = static_cast<float>(j) / sectorCount;
            float v = static_cast<float>(i) / stackCount;

            vertices.push_back({
                position,
                normal,
                glm::vec2(u, v)
            });
        }
    }

    for (uint32_t i = 0; i < stackCount; ++i) {
        uint32_t k1 = i * (sectorCount + 1);
        uint32_t k2 = k1 + sectorCount + 1;

        for (uint32_t j = 0; j < sectorCount; ++j, ++k1, ++k2) {
            if (i != 0) {
                indices.push_back(k1);
                indices.push_back(k2);
                indices.push_back(k1 + 1);
            }

            if (i != stackCount - 1) {
                indices.push_back(k1 + 1);
                indices.push_back(k2);
                indices.push_back(k2 + 1);
            }
        }
    }
}

Sphere::Sphere(Material mat, float radius)
    : Renderable(mat, radius) {
    std::vector<Vertex> vertices;
    std::vector<uint32_t> indices;
    generateSphere(256, 256, vertices, indices);
    setupMesh(vertices, indices);
}

void Sphere::draw(const Camera& cam) {
    bindMaterial(cam);
    glBindVertexArray(VAO);
    glDrawElements(GL_TRIANGLES, indexCount, GL_UNSIGNED_INT, 0);
    glBindVertexArray(0);
}
