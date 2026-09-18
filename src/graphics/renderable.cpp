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
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/quaternion.hpp>
#include <cmath>
#include <cstdint>
#include <numbers>

#ifndef offsetof
#define offsetof(t, d) __builtin_offsetof(t, d)
#endif

static const std::vector<Vertex> skyboxVertices = {
    { { -1.0f,  1.0f, -1.0f } }, { { -1.0f, -1.0f, -1.0f } }, { { 1.0f, -1.0f, -1.0f } },
    { { 1.0f, -1.0f, -1.0f } },  { { 1.0f,  1.0f, -1.0f } },  { { -1.0f,  1.0f, -1.0f } },

    { { -1.0f, -1.0f,  1.0f } }, { { -1.0f, -1.0f, -1.0f } }, { { -1.0f,  1.0f, -1.0f } },
    { { -1.0f,  1.0f, -1.0f } }, { { -1.0f,  1.0f,  1.0f } }, { { -1.0f, -1.0f,  1.0f } },

    { { 1.0f, -1.0f, -1.0f } },  { { 1.0f, -1.0f,  1.0f } },  { { 1.0f,  1.0f,  1.0f } },
    { { 1.0f,  1.0f,  1.0f } },  { { 1.0f,  1.0f, -1.0f } },  { { 1.0f, -1.0f, -1.0f } },

    { { -1.0f, -1.0f,  1.0f } }, { { -1.0f,  1.0f,  1.0f } }, { { 1.0f,  1.0f,  1.0f } },
    { { 1.0f,  1.0f,  1.0f } },  { { 1.0f, -1.0f,  1.0f } },  { { -1.0f, -1.0f,  1.0f } },

    { { -1.0f,  1.0f, -1.0f } }, { { 1.0f,  1.0f, -1.0f } },  { { 1.0f,  1.0f,  1.0f } },
    { { 1.0f,  1.0f,  1.0f } },  { { -1.0f,  1.0f,  1.0f } }, { { -1.0f,  1.0f, -1.0f } },

    { { -1.0f, -1.0f, -1.0f } }, { { -1.0f, -1.0f,  1.0f } }, { { 1.0f, -1.0f, -1.0f } },
    { { 1.0f, -1.0f, -1.0f } },  { { -1.0f, -1.0f,  1.0f } }, { { 1.0f, -1.0f,  1.0f } }
};

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

// helper for skybox
static std::vector<uint32_t> makeIdentityIndices(size_t count) {
    std::vector<uint32_t> idx(count);
    for (size_t i = 0; i < count; i++) idx[i] = static_cast<uint32_t>(i);
    return idx;
}

// helper to generate sphere vertices/indices
static void generateSphere(uint32_t sectorCount, uint32_t stackCount, std::vector<Vertex>& vertices, std::vector<uint32_t>& indices) {
    vertices.clear();
    indices.clear();
    static const float radius = 1.0f;
    static constexpr double pi = std::numbers::pi;

    for (uint32_t i = 0; i <= stackCount; ++i) {
        float stackAngle = pi / 2.0f - i * pi / stackCount;
        float xy = cosf(stackAngle);
        float z = sinf(stackAngle);

        for (uint32_t j = 0; j <= sectorCount; ++j) {
            float sectorAngle = j * 2.0f * pi / sectorCount;

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

// helper to generate cylinder vertices/indices
// Z is the vertical axis. Origin is the center of the TOP face:
// top ring/cap at z = 0, bottom ring/cap at z = -height.
static void generateCylinder(uint32_t sectorCount, std::vector<Vertex>& vertices, std::vector<uint32_t>& indices, bool renderBottom) {
    static const float radius = 1.0f;
    static const float height = 1.0f;
    static constexpr double pi = std::numbers::pi;

    // Side walls
    for (int i = 0; i <= sectorCount; ++i) {
        float angle = 2.0f * pi * i / sectorCount;
        float x = std::cos(angle);
        float y = std::sin(angle);

        // Subdivide into bottom (z = -height) and top (z = 0) ring vertices
        for (int ring = 0; ring < 2; ++ring) {
            float z = (ring == 0) ? -height : 0.0f;

            Vertex v;
            v.pos[0] = x * radius;
            v.pos[1] = y * radius;
            v.pos[2] = z;

            // Side normals point straight outward from the center axis
            v.normal[0] = x;
            v.normal[1] = y;
            v.normal[2] = 0.0f;

            v.uv[0] = (float)i / sectorCount;
            v.uv[1] = (float)ring;

            vertices.push_back(v);
        }
    }

    // Indices for side wall triangles (stitching the two rings together)
    for (int i = 0; i < sectorCount; ++i) {
        uint32_t b0 = i * 2;       // Current bottom vertex
        uint32_t t0 = b0 + 1;      // Current top vertex
        uint32_t b1 = b0 + 2;      // Next bottom vertex
        uint32_t t1 = b0 + 3;      // Next top vertex

        // Triangle 1
        indices.push_back(b1);
        indices.push_back(t0);
        indices.push_back(b0);

        // Triangle 2
        indices.push_back(t1);
        indices.push_back(t0);
        indices.push_back(b1);
    }

    if (renderBottom) {
        // Bottom cap
        unsigned int bottomCenterIndex = vertices.size();

        // Center vertex
        Vertex bottomCenter = {{0.0f, 0.0f, -height}, {0.0f, 0.0f, -1.0f}, {0.5f, 0.5f}};
        vertices.push_back(bottomCenter);

        // Ring vertices for bottom cap
        for (int i = 0; i < sectorCount; ++i) {
            float angle = 2.0f * pi * i / sectorCount;
            float x = std::cos(angle);
            float y = std::sin(angle);

            Vertex v;
            v.pos[0] = x * radius;
            v.pos[1] = y * radius;
            v.pos[2] = -height;
            v.normal[0] = 0.0f; v.normal[1] = 0.0f; v.normal[2] = -1.0f; // Pointing down
            v.uv[0] = x * 0.5f + 0.5f;
            v.uv[1] = y * 0.5f + 0.5f;
            vertices.push_back(v);
        }

        // Bottom triangles
        for (int i = 0; i < sectorCount; ++i) {
            uint32_t current = bottomCenterIndex + 1 + i;
            uint32_t next = bottomCenterIndex + 1 + ((i + 1) % sectorCount);

            indices.push_back(bottomCenterIndex);
            indices.push_back(current);
            indices.push_back(next);
        }
    }

    // Top cap
    unsigned int topCenterIndex = vertices.size();

    // Center vertex (this is the origin)
    Vertex topCenter = {{0.0f, 0.0f, 0.0f}, {0.0f, 0.0f, 1.0f}, {0.5f, 0.5f}};
    vertices.push_back(topCenter);

    // Ring vertices for top cap
    for (int i = 0; i < sectorCount; ++i) {
        float angle = 2.0f * pi * i / sectorCount;
        float x = std::cos(angle);
        float y = std::sin(angle);

        Vertex v;
        v.pos[0] = x * radius;
        v.pos[1] = y * radius;
        v.pos[2] = 0.0f;
        v.normal[0] = 0.0f; v.normal[1] = 0.0f; v.normal[2] = 1.0f; // Pointing up
        v.uv[0] = x * 0.5f + 0.5f;
        v.uv[1] = y * 0.5f + 0.5f;
        vertices.push_back(v);
    }

    // Top triangles
    for (int i = 0; i < sectorCount; ++i) {
        unsigned int current = topCenterIndex + 1 + i;
        unsigned int next = topCenterIndex + 1 + ((i + 1) % sectorCount);

        indices.push_back(topCenterIndex);
        indices.push_back(next);
        indices.push_back(current);
    }
}

// MESH

Mesh::Mesh(const std::vector<Vertex>& vertices, const std::vector<uint32_t>& indices, int materialIndex)
    : materialIndex(materialIndex), indexCount(static_cast<GLsizei>(indices.size())) {
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

Mesh::Mesh(Mesh&& other) noexcept
    : materialIndex(other.materialIndex), VAO(other.VAO), VBO(other.VBO), EBO(other.EBO),
      indexCount(other.indexCount) {
    other.VAO = other.VBO = other.EBO = 0;
    other.indexCount = 0;
}

Mesh& Mesh::operator=(Mesh&& other) noexcept {
    if (this != &other) {
        glDeleteVertexArrays(1, &VAO);
        glDeleteBuffers(1, &VBO);
        glDeleteBuffers(1, &EBO);

        materialIndex = other.materialIndex;
        VAO = other.VAO;
        VBO = other.VBO;
        EBO = other.EBO;
        indexCount = other.indexCount;

        other.VAO = other.VBO = other.EBO = 0;
        other.indexCount = 0;
    }
    return *this;
}

Mesh::~Mesh() {
    glDeleteVertexArrays(1, &VAO);
    glDeleteBuffers(1, &VBO);
    glDeleteBuffers(1, &EBO);
}

void Mesh::draw() const {
    glBindVertexArray(VAO);
    glDrawElements(GL_TRIANGLES, indexCount, GL_UNSIGNED_INT, 0);
    glBindVertexArray(0);
}

// RENDERABLE

Renderable::Renderable(const std::string& id, Material mat, glm::vec3 renderScale, glm::dvec3 realOffset)
    : id(id), renderScale(renderScale), realOffset(realOffset) {
    materials.push_back(std::move(mat));
}

Renderable::Renderable(const std::string& id, std::vector<Material> mats, glm::vec3 renderScale, glm::dvec3 realOffset)
    : id(id), materials(std::move(mats)), renderScale(renderScale), realOffset(realOffset) {
}

void Renderable::initDebugGeometry(const Collider* coll) {
    std::vector<Vertex> vertices;
    std::vector<uint32_t> indices;
    if (auto* sphereColl = dynamic_cast<const SphereCollider*>(coll)) {
        generateSphere(256, 256, vertices, indices);
    } else if (auto* obbColl = dynamic_cast<const OBBCollider*>(coll)) {
        vertices = {
            {{-0.5f, -0.5f, -0.5f}}, // 0
            {{ 0.5f, -0.5f, -0.5f}}, // 1
            {{ 0.5f,  0.5f, -0.5f}}, // 2
            {{-0.5f,  0.5f, -0.5f}}, // 3
            {{-0.5f, -0.5f,  0.5f}}, // 4
            {{ 0.5f, -0.5f,  0.5f}}, // 5
            {{ 0.5f,  0.5f,  0.5f}}, // 6
            {{-0.5f,  0.5f,  0.5f}}  // 7
        };
        indices = {
            0,1, 1,2, 2,3, 3,0, // Bottom square
            4,5, 5,6, 6,7, 7,4, // Top square
            0,4, 1,5, 2,6, 3,7  // Connecting pillars
        };
    }
    debugMesh = Mesh(vertices, indices);
    debugInitialized = true;
}

void Renderable::drawDebug(const Simulation& sim, const Camera& cam) {
    if (!sim.colliders.contains(id)) return;
    Collider* coll = sim.colliders.at(id).get();
    if (!debugInitialized) initDebugGeometry(coll);

    Shader entityShader = materials[0].shader; // Should be entity?
    entityShader.setMat4("view", cam.view);
    entityShader.setMat4("projection", cam.projection);
    entityShader.setBool("uIsDebug", true);

    glDisable(GL_CULL_FACE);
    glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

    glm::mat4 collModel = glm::translate(glm::mat4(1.0f), toRenderUnits(coll->centerPos - cam.realPos));
    collModel = collModel * glm::mat4_cast(glm::quat(coll->rot));

    if (auto* sphereColl = dynamic_cast<const SphereCollider*>(coll)) {
        collModel = glm::scale(collModel, glm::vec3(toRenderUnits(sphereColl->radius)));
        entityShader.setMat4("model", collModel);

        glBindVertexArray(debugMesh.VAO);
        glDrawElements(GL_TRIANGLES, debugMesh.indexCount, GL_UNSIGNED_INT, 0);
    } else if (auto* obbColl = dynamic_cast<const OBBCollider*>(coll)) {
        collModel = glm::scale(collModel, toRenderUnits(obbColl->halfExtent) * 2.0f);
        entityShader.setMat4("model", collModel);
        
        glBindVertexArray(debugMesh.VAO);
        glDrawElements(GL_LINES, debugMesh.indexCount, GL_UNSIGNED_INT, 0);
    }

    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    glEnable(GL_CULL_FACE);
    glBindVertexArray(0);
}

void Renderable::updateFromRigidBody(const RigidBody& rb) {
    realPos = rb.pos + glm::dquat(rb.rot) * realOffset;
    rotation = glm::mat4_cast(rb.rot);
}

void Renderable::setModel(const glm::mat4& m) {
    model = m;
}

glm::mat4& Renderable::getModel() {
    return model;
}

void Renderable::setSunRenderPos(const glm::vec3& sunRenderPos) {
    for (auto& m : materials) m.uSunPos = sunRenderPos;
}

// MODEL

static glm::mat4 aiToGlm(const aiMatrix4x4& m) {
    return glm::transpose(glm::make_mat4(&m.a1));
}

Model::Model(const std::string& id, const std::string& path, Material materialTemplate, glm::vec3 renderScale, glm::dvec3 realOffset)
    : Renderable(id, std::vector<Material>{}, renderScale, realOffset), materialTemplate(std::move(materialTemplate)) {
    loadModel(path);
}

void Model::draw(const Simulation& sim, const Camera& cam) {
    for (const Mesh& mesh : meshes) {
        const Material& mat = materials.at(mesh.materialIndex);

        mat.shader.use();
        mat.shader.setMat4("model", model);
        mat.shader.setMat4("view", cam.view);
        mat.shader.setMat4("projection", cam.projection);
        mat.shader.setBool("uIsDebug", mat.uIsDebug);
        mat.shader.setVec4("uBaseColor", mat.uBaseColor);
        mat.shader.setInt("uTextureMap", mat.uTextureMap);
        mat.shader.setBool("uUseTexture", mat.uUseTexture);
        mat.shader.setVec3("uSunPos", mat.uSunPos);
        mat.shader.setVec3("uAmbientLighting", mat.uAmbientLighting);
        mat.shader.setVec3("uEmissiveLighting", mat.uEmissiveLighting);
        mat.shader.setBool("uUseRaytracedSphere", false);
        mat.shader.setBool("uUseDayNightBlend", false);
        mat.shader.setVec3("uNightAmbientBoost", glm::vec3(0.0f));

        if (mat.uUseTexture) {
            glActiveTexture(GL_TEXTURE0);
            glBindTexture(GL_TEXTURE_2D, mat.textureID);
        }

        mesh.draw();
    }
}

void Model::loadModel(const std::string& path) {
    Assimp::Importer importer;
    const aiScene* scene = importer.ReadFile(path,
        aiProcess_Triangulate |
        aiProcess_GenSmoothNormals |
        aiProcess_JoinIdenticalVertices |
        aiProcess_CalcTangentSpace |
        aiProcess_FlipUVs);

    if (!scene || (scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE) || !scene->mRootNode) {
        fprintf(stderr, "Assimp error loading '%s': %s\n", path.c_str(), importer.GetErrorString());
        return;
    }

    size_t slash = path.find_last_of('/');
    directory = (slash == std::string::npos) ? "" : path.substr(0, slash);

    std::vector<RawMesh> raw;
    processNode(scene->mRootNode, scene, glm::mat4(1.0f), raw);
    normalizeAndUpload(raw);
}

void Model::processNode(aiNode* node, const aiScene* scene, const glm::mat4& parentTransform, std::vector<RawMesh>& out) {
    glm::mat4 nodeTransform = parentTransform * aiToGlm(node->mTransformation);

    for (unsigned int i = 0; i < node->mNumMeshes; i++) {
        aiMesh* mesh = scene->mMeshes[node->mMeshes[i]];
        processMesh(mesh, scene, nodeTransform, out);
    }
    for (unsigned int i = 0; i < node->mNumChildren; i++) {
        processNode(node->mChildren[i], scene, nodeTransform, out);
    }
}

void Model::processMesh(aiMesh* mesh, const aiScene* scene, const glm::mat4& transform, std::vector<RawMesh>& out) {
    std::vector<Vertex> vertices;
    vertices.reserve(mesh->mNumVertices);

    glm::mat3 normalMatrix = glm::mat3(glm::transpose(glm::inverse(transform)));

    for (size_t i = 0; i < mesh->mNumVertices; i++) {
        glm::vec4 pos = transform * glm::vec4(
            mesh->mVertices[i].x, mesh->mVertices[i].y, mesh->mVertices[i].z, 1.0f);

        glm::vec3 normal(0.0f, 1.0f, 0.0f);
        if (mesh->HasNormals()) {
            normal = normalMatrix * glm::vec3(
                mesh->mNormals[i].x, mesh->mNormals[i].y, mesh->mNormals[i].z);
        }

        glm::vec2 uv(0.0f);
        if (mesh->mTextureCoords[0]) {
            uv = glm::vec2(mesh->mTextureCoords[0][i].x, mesh->mTextureCoords[0][i].y);
        }

        vertices.push_back({ glm::vec3(pos), glm::normalize(normal), uv });
    }

    std::vector<uint32_t> indices;
    indices.reserve(mesh->mNumFaces * 3);
    for (size_t i = 0; i < mesh->mNumFaces; i++) {
        const aiFace& face = mesh->mFaces[i];
        for (size_t j = 0; j < face.mNumIndices; j++)
            indices.push_back(face.mIndices[j]);
    }

    int materialIndex = 0;
    if (mesh->mMaterialIndex >= 0 && scene->mMaterials) {
        materialIndex = processMaterial(scene->mMaterials[mesh->mMaterialIndex], scene);
    }
    
    out.push_back({ std::move(vertices), std::move(indices), materialIndex });
}

int Model::processMaterial(aiMaterial* aiMat, const aiScene* scene) {
    Material m = materialTemplate; // copy: shader + default uniforms carried over

    aiString texPath;
    if (aiMat->GetTextureCount(aiTextureType_DIFFUSE) > 0 &&
        aiMat->GetTexture(aiTextureType_DIFFUSE, 0, &texPath) == AI_SUCCESS) {

        const char* p = texPath.C_Str();

        if (p[0] == '*') {
            // Embedded texture (common in .glb): "*0", "*1", ...
            int embeddedIndex = std::atoi(p + 1);
            const aiTexture* tex = scene->mTextures[embeddedIndex];
            if (tex->mHeight == 0) {
                // Compressed (png/jpg) blob, length in mWidth
                m.textureID = GraphicsEngine::loadTextureFromMemory(
                    reinterpret_cast<const uint8_t*>(tex->pcData), tex->mWidth, GL_CLAMP_TO_EDGE);
            } else {
                // Uncompressed raw texel data — rare in practice for glTF, but handle it
                m.textureID = GraphicsEngine::loadTextureFromRawRGBA(
                    reinterpret_cast<const uint8_t*>(tex->pcData), tex->mWidth, tex->mHeight);
            }
        } else {
            std::string fullPath = directory.empty() ? p : directory + "/" + p;
            m.textureID = GraphicsEngine::loadTextureFromFile(fullPath, GL_CLAMP_TO_EDGE);
        }
        m.uUseTexture = true;
    }

    materials.push_back(m);
    return static_cast<int>(materials.size() - 1);
}

void Model::normalizeAndUpload(std::vector<RawMesh>& raw) {
    glm::vec3 mn(std::numeric_limits<float>::max());
    glm::vec3 mx(std::numeric_limits<float>::lowest());

    for (const auto& rm : raw)
        for (const auto& v : rm.vertices) {
            mn = glm::min(mn, v.pos);
            mx = glm::max(mx, v.pos);
        }

    glm::vec3 center = (mn + mx) * 0.5f;
    glm::vec3 extent = mx - mn;
    float maxExtent = std::max({ extent.x, extent.y, extent.z });
    float scaleFactor = (maxExtent > 1e-8f) ? (1.0f / maxExtent) : 1.0f;

    for (auto& rm : raw) {
        for (auto& v : rm.vertices) {
            v.pos = (v.pos - center) * scaleFactor;
        }
        meshes.emplace_back(std::move(rm.vertices), std::move(rm.indices), rm.materialIndex);
    }
}

// SKYBOX

SkyBox::SkyBox(const std::string& id, Material mat) : Renderable(id, std::move(mat)) {
    meshes.emplace_back(skyboxVertices, makeIdentityIndices(skyboxVertices.size()), 0);
}

void SkyBox::draw(const Simulation& sim, const Camera& cam) {
    const Mesh& mesh = meshes[0];
    const Material& mat = materials[mesh.materialIndex];

    glDepthFunc(GL_GEQUAL);
    glDepthMask(GL_FALSE);

    mat.shader.use();
    mat.shader.setMat4("view", glm::mat4(glm::mat3(cam.view)));
    mat.shader.setMat4("projection", cam.projection);

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_CUBE_MAP, mat.textureID);

    mesh.draw();

    glDepthMask(GL_TRUE);
    glDepthFunc(GL_GREATER);
}

// CUBE

Cube::Cube(const std::string& id, Material mat, glm::vec3 renderScale, glm::dvec3 realOffset)
    : Renderable(id, mat, renderScale, realOffset) {
    meshes.emplace_back(cubeVertices, cubeIndices, 0);
}

void Cube::draw(const Simulation& sim, const Camera& cam) {
    const Mesh& mesh = meshes[0];
    const Material& mat = materials[mesh.materialIndex];

    mat.shader.use();
    mat.shader.setMat4("model", model);
    mat.shader.setMat4("view", cam.view);
    mat.shader.setMat4("projection", cam.projection);
    mat.shader.setBool("uIsDebug", mat.uIsDebug);
    mat.shader.setVec4("uBaseColor", mat.uBaseColor);
    mat.shader.setInt("uTextureMap", mat.uTextureMap);
    mat.shader.setBool("uUseTexture", mat.uUseTexture);
    mat.shader.setVec3("uSunPos", mat.uSunPos);
    mat.shader.setVec3("uAmbientLighting", mat.uAmbientLighting);
    mat.shader.setVec3("uEmissiveLighting", mat.uEmissiveLighting);
    mat.shader.setBool("uUseRaytracedSphere", false);
    mat.shader.setBool("uUseDayNightBlend", false);
    mat.shader.setVec3("uNightAmbientBoost", glm::vec3(0.0f));

    if (mat.uUseTexture) {
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, mat.textureID);
    }

    mesh.draw();
}

// CELESTIAL BODY 

CelestialBody::CelestialBody(const std::string& id, Material mat, glm::vec3 renderScale, glm::dvec3 realOffset)
    : Renderable(id, mat, renderScale, realOffset) {
    std::vector<Vertex> vertices;
    std::vector<uint32_t> indices;
    generateSphere(512, 512, vertices, indices);
    meshes.emplace_back(std::move(vertices), std::move(indices), 0);
}

void CelestialBody::draw(const Simulation& sim, const Camera& cam) {
    const Mesh& mesh = meshes[0];
    const Material& mat = materials[mesh.materialIndex];

    mat.shader.use();
    mat.shader.setMat4("model", model);
    mat.shader.setMat4("view", cam.view);
    mat.shader.setMat4("projection", cam.projection);
    mat.shader.setBool("uIsDebug", mat.uIsDebug);
    mat.shader.setVec4("uBaseColor", mat.uBaseColor);
    mat.shader.setInt("uTextureMap", mat.uTextureMap);
    mat.shader.setBool("uUseTexture", mat.uUseTexture);
    mat.shader.setInt("uNightTextureMap", mat.uNightTextureMap);
    mat.shader.setBool("uUseDayNightBlend", mat.uUseDayNightBlend);
    mat.shader.setVec3("uSunPos", mat.uSunPos);
    mat.shader.setVec3("uAmbientLighting", mat.uAmbientLighting);
    mat.shader.setVec3("uNightAmbientBoost", mat.uNightAmbientBoost);
    mat.shader.setVec3("uEmissiveLighting", mat.uEmissiveLighting);
    mat.shader.setBool("uUseRaytracedSphere", mat.uUseRaytracedSphere);

    if (mat.uUseTexture) {
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, mat.textureID);
    }

    if (mat.uUseDayNightBlend) {
        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_2D, mat.textureID2);
    }

    mesh.draw();
}

// EXHAUST PLUME

ExhaustPlume::ExhaustPlume(const std::string& id, Material mat, glm::vec3 renderScale, glm::dvec3 realOffset, ExhaustConfig cfg)
    : Renderable(id, std::move(mat), renderScale, realOffset), config(cfg) {
    std::vector<Vertex> vertices;
    std::vector<uint32_t> indices;

    generateCylinder(64, vertices, indices, false);
    meshes.emplace_back(std::move(vertices), std::move(indices), 0);
}

void ExhaustPlume::draw(const Simulation& sim, const Camera& cam) {
    size_t delimPos = id.find(':');
    if (delimPos == std::string::npos) return;

    const std::string mainID = id.substr(0, delimPos);
    const std::string thrusterID = id.substr(delimPos + 1);

    if (!sim.controllers.contains(mainID)) return;

    auto& controller = sim.controllers.at(mainID);
    if (!controller->thrusters.contains(thrusterID)) return;

    Thruster& thruster = controller->thrusters.at(thrusterID);
    if (thruster.thrust <= 0.0f) return;

    const Mesh& mesh = meshes[0];
    const Material& mat = materials[mesh.materialIndex];

    constexpr float maxThrust = 7.44e7f;
    float thrustFactor = glm::clamp(static_cast<float>(thruster.thrust) / maxThrust, 0.0f, 1.0f);

    float plumeLength = glm::mix(config.lengthIdle, config.lengthFull, thrustFactor);
    float plumeRadius = glm::mix(config.radiusIdle, config.radiusFull, thrustFactor);

    glm::vec3 localExhaustDir = -glm::vec3(thruster.dir);
    if (glm::dot(localExhaustDir, localExhaustDir) < 1e-8f) {
        localExhaustDir = glm::vec3(0.0f, -1.0f, 0.0f);
    } else {
        localExhaustDir = glm::normalize(localExhaustDir);
    }

    glm::vec3 exhaustDir = glm::normalize(glm::vec3(rotation * glm::vec4(localExhaustDir, 0.0f)));
    glm::quat cylinderAlignment = glm::rotation(glm::vec3(0.0f, 0.0f, -1.0f), localExhaustDir);

    glm::mat4 plumeModel = glm::translate(glm::mat4(1.0f), renderPos) * 
                           rotation * 
                           glm::mat4_cast(cylinderAlignment) * 
                           glm::scale(glm::mat4(1.0f), glm::vec3(plumeRadius, plumeRadius, plumeLength));

    mat.shader.use();
    mat.shader.setMat4("model", plumeModel);
    mat.shader.setMat4("view", cam.view);
    mat.shader.setMat4("projection", cam.projection);
    mat.shader.setFloat("uThrust", static_cast<float>(thruster.thrust));
    mat.shader.setVec3("uThrustDir", exhaustDir);
    mat.shader.setFloat("uTime", static_cast<float>(glfwGetTime()));

    // Cone shape & dynamics uniforms
    mat.shader.setFloat("uExpansionRate", config.expansionRate);
    mat.shader.setFloat("uExpansionPower", config.expansionPower);
    mat.shader.setInt("uDiamondCount", config.diamondCount);
    mat.shader.setFloat("uNeckStrength", config.neckStrength);
    mat.shader.setFloat("uStreakSpeed", config.streakSpeed);
    mat.shader.setFloat("uStreakSharpness", config.streakSharpness);

    // Color uniforms
    mat.shader.setVec3("uCoreColorCold", config.coreColorCold);
    mat.shader.setVec3("uCoreColorHot", config.coreColorHot);
    mat.shader.setVec3("uMachColor", config.machColor);
    mat.shader.setVec3("uFringeColor", config.fringeColor);
    mat.shader.setFloat("uFringeAlphaScale", config.fringeAlphaScale);
    mat.shader.setFloat("uFringeSpread", config.fringeSpread);

    glEnable(GL_DEPTH_TEST);
    glDepthMask(GL_FALSE);
    glDisable(GL_CULL_FACE);
    glEnable(GL_BLEND);
    glBlendFunci(0, GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glBlendFunci(1, GL_ONE, GL_ONE);

    // Draw passes (Outer fringe, Mach sheath, Inner core)
    mat.shader.setInt("uRenderPass", 2);
    mesh.draw();

    mat.shader.setInt("uRenderPass", 1);
    mesh.draw();

    mat.shader.setInt("uRenderPass", 0);
    mesh.draw();

    glDisable(GL_BLEND);
    glDepthMask(GL_TRUE);
    glEnable(GL_CULL_FACE);
}
