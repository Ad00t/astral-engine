#ifndef RENDERABLE_H
#define RENDERABLE_H

#include "assimp/scene.h"
#include "glm/ext/vector_float3.hpp"
#include "opengl_includes.h"
#include "graphics/shader.h"
#include "physics/collider.h"
#include "core/controller.h"
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <vector>
#include <memory>
#include <cstdint>

class Camera;
class Simulation;

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
   
    bool uIsDebug = false;
    glm::vec4 uBaseColor = glm::vec4(1.0f);
    bool uUseTexture = false;
    int uTextureMap = 0;
    bool uUseDayNightBlend = false;
    int uNightTextureMap = 1;
    glm::vec3 uSunPos = glm::vec3(0.0f);
    glm::vec3 uAmbientLighting = glm::vec3(0.02f);
    glm::vec3 uNightAmbientBoost = glm::vec3(0.8f);
    glm::vec3 uEmissiveLighting = glm::vec3(0.0f);
    bool uUseRaytracedSphere = false;
    float uLunarLambertWeight = 0.1;
    
    AtmosphereParams atmosphere = {};

    float uThrust = 0;
    float uTime = 0; 
    glm::vec3 uThrustSrc = glm::vec3(0.0f);
    glm::vec3 uThrustDir = glm::vec3(0.0f);
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

    GLuint VAO = 0, VBO = 0, EBO = 0;
    GLsizei indexCount = 0;
};

class Renderable {
private:
    bool debugInitialized = false;
    Mesh debugMesh;

    void initDebugGeometry(const Collider* coll);

protected:
    std::vector<Mesh> meshes;
    std::vector<Material> materials;
    glm::mat4 model = glm::mat4(1.0f);

public:
    std::string id;
    glm::dvec3 realOffset = glm::dvec3(0.0);
    glm::dvec3 realPos = glm::dvec3(0.0);
    glm::vec3 renderPos = glm::vec3(0.0f);
    glm::mat4 rotation = glm::mat4(1.0f);
    glm::vec3 renderScale = glm::vec3(1.0f);

    Renderable(const std::string& id, Material mat, glm::vec3 renderScale = glm::vec3(1.0f), glm::dvec3 realOffset = glm::dvec3(0.0)); // Single-material shapes              
    Renderable(const std::string& id, std::vector<Material> mats, glm::vec3 renderScale = glm::vec3(1.0f), glm::dvec3 realOffset = glm::dvec3(0.0)); // Models
    Renderable(const Renderable&) = delete;
    Renderable& operator=(const Renderable&) = delete;
    Renderable(Renderable&&) noexcept = default;
    Renderable& operator=(Renderable&&) noexcept = default;
    virtual ~Renderable() = default;

    virtual void draw(const Camera& cam) = 0; 
    void drawDebug(const Camera& cam, const Collider* coll); 
    void updateFromRigidBody(RigidBody* rb);
    void setModel(const glm::mat4& m);
    glm::mat4& getModel();
    const Material& getMaterial(size_t i = 0) const { return materials.at(i); }
    void setSunRenderPos(const glm::vec3& sunPos); // now sets it on every material
};

class Model : public Renderable {
private:
    struct RawMesh {
        std::vector<Vertex> vertices;
        std::vector<uint32_t> indices;
        int materialIndex;
    };

    void loadModel(const std::string& path);
    void processNode(aiNode* node, const aiScene* scene, const glm::mat4& parentTransform, std::vector<RawMesh>& out);
    void processMesh(aiMesh* mesh, const aiScene* scene, const glm::mat4& transform, std::vector<RawMesh>& out);
    int processMaterial(aiMaterial* aiMat, const aiScene* scene);
    void normalizeAndUpload(std::vector<RawMesh>& raw);

    std::string directory;
    Material materialTemplate; // supplies shader + default uniforms (ambient, atmosphere off, etc.)

public:
    Model(const std::string& id, const std::string& path, Material materialTemplate, glm::vec3 renderScale = glm::vec3(1.0f), glm::dvec3 realOffset = glm::dvec3(0.0));
    void draw(const Camera& cam) override;
};

class SkyBox : public Renderable {
public:
    SkyBox(const std::string& id, Material mat);
    void draw(const Camera& cam) override;
};

class Cube : public Renderable {
public:
    Cube(const std::string& id, Material mat, glm::vec3 renderScale = glm::vec3(1.0f), glm::dvec3 realOffset = glm::dvec3(0.0));
    void draw(const Camera& cam) override;
};

class CelestialBody : public Renderable {
public:
    CelestialBody(const std::string& id, Material mat, glm::vec3 renderScale = glm::vec3(1.0f), glm::dvec3 realOffset = glm::dvec3(0.0)); 
    void draw(const Camera& cam) override;
};


struct ExhaustConfig {
    // --- Cone size ---
    float lengthIdle = 0.05f;
    float lengthFull = 0.25f;
    float radiusIdle = 0.0045f;
    float radiusFull = 0.006f;

    // --- Cone shape & shock diamonds ---
    float expansionRate  = 1.6f;
    float expansionPower = 1.65f;
    int   diamondCount   = 4;
    float neckStrength   = 0.15f;

    // --- Streak look ---
    float streakSpeed     = 50.0f;
    float streakSharpness = 10.0f;

    // --- Colors ---
    glm::vec3 coreColorCold = {1.20f, 1.80f, 2.50f}; 
    glm::vec3 coreColorHot  = {3.00f, 0.15f, 2.80f}; 
    glm::vec3 machColor     = {0.05f, 0.25f, 1.50f}; 
    glm::vec3 fringeColor   = {0.05f, 0.01f, 0.15f};
    
    // --- Fringe ---
    float fringeAlphaScale  = 1.0f;
    float fringeSpread      = 0.7f;
};

class ExhaustPlume : public Renderable {
public:
    Thruster* thruster;

    ExhaustPlume(const std::string& id, Material mat, glm::vec3 renderScale, Thruster* thruster, ExhaustConfig cfg = ExhaustConfig{});
    ~ExhaustPlume() override = default;

    void draw(const Camera& cam) override;

    ExhaustConfig config;
};


#endif // RENDERABLE_H
