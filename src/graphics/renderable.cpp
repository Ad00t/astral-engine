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

Renderable::Renderable(Material mat, float radius)
    : VAO(0), VBO(0), EBO(0), mat(mat), radius(radius), model(glm::mat4(1.0f)), indexCount(0), 
      vertices(std::vector<Vertex>()), indices(std::vector<uint32_t>()) {
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

void Renderable::setModel(const glm::mat4& m) {
    model = m;
}

glm::mat4& Renderable::getModel() {
    return model;
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
    glm::mat4 viewNoTranslation = glm::mat4(glm::mat3(cam.view)); // strip translation, keep rotation

    glDepthFunc(GL_LEQUAL);
    mat.shader.use();
    mat.shader.setMat4("view", viewNoTranslation);
    mat.shader.setMat4("projection", cam.projection);

    glBindVertexArray(VAO);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_CUBE_MAP, mat.textureID);
    glDrawArrays(GL_TRIANGLES, 0, 36);
    glBindVertexArray(0);
    glDepthFunc(GL_LESS);
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

Cube::Cube(Material mat, float sideLength)
    : Renderable(mat, sideLength) {
    setupMesh(cubeVertices, cubeIndices);
}

void Cube::draw(const Camera& cam) {
    mat.shader.use();
    mat.shader.setMat4("model", model);
    mat.shader.setMat4("view", cam.view);
    mat.shader.setMat4("projection", cam.projection);
    mat.shader.setVec4("uMaterialColor", glm::vec4(mat.baseColor, 1.0)); 
    
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

Sphere::Sphere(Material mat, float radius)
    : Renderable(mat, radius) {
    std::vector<Vertex> vertices;
    std::vector<uint32_t> indices;
    generateSphere(radius, 18, 36, vertices, indices);
    setupMesh(vertices, indices);
}

void Sphere::draw(const Camera& cam) {
    mat.shader.use();
    mat.shader.setMat4("model", model);
    mat.shader.setMat4("view", cam.view);
    mat.shader.setMat4("projection", cam.projection);
    mat.shader.setVec4("uMaterialColor", glm::vec4(mat.baseColor, 1.0)); 

    mat.shader.setBool("uUseTexture", mat.textureID != -1);
    mat.shader.setInt("uTextureMap", 0);   
    if (mat.textureID != -1) {
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, mat.textureID);
    }

    glBindVertexArray(VAO);
    glDrawElements(GL_TRIANGLES, indexCount, GL_UNSIGNED_INT, 0);
    glBindVertexArray(0);
}
