#ifndef RENDERABLE_H
#define RENDERABLE_H

#include "assimp/scene.h"
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

struct AtmosphereParams {
    bool enabled = false;
    float radiusMultiplier = 3.0f * 1.0157f;                                    // atmosphere shell = planet radius * this
    glm::vec3 rayleighCoeff = glm::vec3(5.5e-6f, 13.0e-6f, 22.4e-6f) / 1e-2f;   // per meter, sea level
    float mieCoeff = 21e-6f / 1e-2f;                                            // per meter, sea level
    float rayleighScaleHeight = 8500.0f * 1e-2f;                                // meters
    float mieScaleHeight = 1200.0f * 1e-2f;                                     // meters
    float mieG = 0.758f;
    int numSamples = 16;
    int numLightSamples = 8;
};

struct Material {
    Shader shader;
    GLuint textureID;
    GLuint textureID2;
   
    // entity params 
    glm::vec4 uBaseColor = glm::vec4(1.0f);
    bool uUseTexture = false;
    int uTextureMap = 0;
    bool uUseDayNightBlend = false;
    int uNightTextureMap = 1;
    glm::vec3 uSunPos = glm::vec3(0.0f);
    glm::vec3 uAmbientLighting = glm::vec3(0.02f);
    glm::vec3 uNightAmbientBoost = glm::vec3(0.8f);
    glm::vec3 uEmissiveLighting = glm::vec3(0.0f);

    // atmosphere params
    AtmosphereParams atmosphere = {};
};

class Mesh {
public:
    Mesh() = default;
    Mesh(const std::vector<Vertex>& vertices, const std::vector<uint32_t>& indices, int materialIndex = 0);
    Mesh(const Mesh&) = delete;
    Mesh& operator=(const Mesh&) = delete;
    Mesh(Mesh&&) noexcept;            
    Mesh& operator=(Mesh&&) noexcept;
    ~Mesh();

    void draw() const;               

    int materialIndex = 0;

private:
    GLuint VAO = 0, VBO = 0, EBO = 0;
    GLsizei indexCount = 0;
};

class Renderable {
protected:
    std::vector<Mesh> meshes;
    std::vector<Material> materials;
    glm::mat4 model = glm::mat4(1.0f);
    void bindMaterial(const Camera& cam, const Material& m); 

public:
    glm::dvec3 realPos = glm::dvec3(0.0);
    glm::vec3 renderPos = glm::vec3(0.0f);
    glm::mat4 rotation = glm::mat4(1.0f);
    float renderScale;

    Renderable(Material mat, float renderScale); // Single-material shapes              
    Renderable(std::vector<Material> mats, float renderScale); // Models
    Renderable(const Renderable&) = delete;
    Renderable& operator=(const Renderable&) = delete;
    Renderable(Renderable&&) noexcept = default;
    Renderable& operator=(Renderable&&) noexcept = default;
    virtual ~Renderable() = default;

    virtual void draw(const Camera& cam); 
    void setModel(const glm::mat4& m);
    glm::mat4& getModel();
    const Material& getMaterial(size_t i = 0) const { return materials.at(i); }
    void setSunRenderPos(const glm::vec3& sunPos); // now sets it on every material
};

class Model : public Renderable {
public:
    Model(const std::string& path, Material materialTemplate, float renderScale);
private:
    void loadModel(const std::string& path);
    void processNode(aiNode* node, const aiScene* scene, const glm::mat4& parentTransform);
    void processMesh(aiMesh* mesh, const aiScene* scene, const glm::mat4& transform);
    int processMaterial(aiMaterial* aiMat, const aiScene* scene);

    std::string directory;
    Material materialTemplate; // supplies shader + default uniforms (ambient, atmosphere off, etc.)
};

class SkyBox : public Renderable {
public:
    SkyBox(Material mat);
    void draw(const Camera& cam) override;
};

class Cube : public Renderable {
public:
    Cube(Material mat, float sideLength);
};

class Sphere : public Renderable {
public:
    Sphere(Material mat, float radius); 
};

#endif // RENDERABLE_H
