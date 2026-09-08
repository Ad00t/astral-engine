#include "core/simulation.h"
#include "glm/trigonometric.hpp"
#include "graphics/camera.h"
#include "graphics/graphics_engine.h"
#include "graphics/renderable.h"        
#include "physics/physics_engine.h"
#include "physics/collider.h"
#include "utils.h"
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/quaternion.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <memory>

constexpr double R_SUN = 7.957e8;
constexpr double M_SUN = 1.989e30;

constexpr double R_EARTH = 6.378e6;
constexpr double M_EARTH = 5.972e24;

constexpr double R_MOON = 1.7375e6;
constexpr double M_MOON = 7.35e22;

Simulation::Simulation(GraphicsEngine& gEng, PhysicsEngine& pEng) {
    runPhysics.store(true);

    // Spacebox
    renderables.emplace("spacebox", std::make_unique<SkyBox>(
        Material{
            .shader = gEng.getShader("skybox"), 
            .textureID = gEng.getTextureID("cubemap/spacebox"),
            .uBaseColor = glm::vec4(0, 0, 0, 1) 
        }
    ));

    // Sun
    rigidbodies.emplace("sun", RigidBody(
        glm::dvec3(0, 0, 0), 
        glm::dvec3(0, 0, 0), 
        glm::quat(0.568933, 0.059228, 0.305817, 0.761107),
        glm::dvec3(3.505831e-7, -8.893373e-8, 2.842410e-6),
        M_SUN 
    ));
    colliders.emplace("sun", std::make_unique<SphereCollider>(
        rigidbodies["sun"],
        0.0,
        R_SUN 
    ));
    renderables.emplace("sun", std::make_unique<Sphere>(
        Material{
            .shader = gEng.getShader("entity"), 
            .textureID = gEng.getTextureID("uvmap/sun"),
            .uBaseColor = glm::vec4(1, 1, 0, 1),
            .uUseTexture = true,
            .uEmissiveLighting = glm::vec3(1000.0f)
        },
        toRenderUnits(colliders["sun"]->getMaxRadius())
    ));

    // Earth
    rigidbodies.emplace("earth", RigidBody(
        glm::dvec3(1.496e11, 0, 0),
        glm::dvec3(0, 3.0e4, 0), 
        glm::quat(0.750882, -0.155769, 0.130365, 0.628425),
        glm::dvec3(4.47e-21, 2.900637e-5, 6.690385e-5),
        M_EARTH
    ));
    colliders.emplace("earth", std::make_unique<SphereCollider>(
        rigidbodies["earth"],
        0.0,
        R_EARTH
    ));
    renderables.emplace("earth", std::make_unique<Sphere>(
        Material{
            .shader = gEng.getShader("entity"), 
            .textureID = gEng.getTextureID("uvmap/earth_day"),
            .textureID2 = gEng.getTextureID("uvmap/earth_night"),
            .uBaseColor = glm::vec4(0, 0, 1, 1), 
            .uUseTexture = true,
            .uUseDayNightBlend = true,
            .atmosphere = AtmosphereParams{ .enabled = true }
        },
        toRenderUnits(colliders["earth"]->getMaxRadius())
    ));
   
    // Moon
    rigidbodies.emplace("moon", RigidBody(
        glm::dvec3(1.496e11 + 3.84e8, 0, 0),
        glm::dvec3(0, 3.0e4 + 1.022e3, 0), 
        glm::quat(0.328257, 0.000045, 0.375904, 0.866570),
        glm::dvec3(-9.432401e-11, -9.992006e-10, 2.661699e-6),
        M_MOON 
    ));
    colliders.emplace("moon", std::make_unique<SphereCollider>(
        rigidbodies["moon"],
        0.0,
        R_MOON 
    ));
    renderables.emplace("moon", std::make_unique<Sphere>(
        Material{
            .shader = gEng.getShader("entity"),
            .textureID = gEng.getTextureID("uvmap/moon"),
            .uBaseColor = glm::vec4(1, 1, 1, 1),
            .uUseTexture = true
        },
        toRenderUnits(colliders["moon"]->getMaxRadius())
    ));

    // ISS
    rigidbodies.emplace("iss", RigidBody(
        rigidbodies["earth"],
        OrbitElements{
            .a = 6.7975e6,
            .e = 0.0004984,
            .i = glm::radians(51.6306),
            .w = glm::radians(115.8922),
            .raan = glm::radians(252.7093),
            .nu = glm::radians(0.0)
        },
        glm::quat(1, 0, 0, 0),
        glm::dvec3(0, 0, 0),
        450000
    ));
    colliders.emplace("iss", std::make_unique<OBBCollider>(
        rigidbodies["iss"],
        0.0,
        glm::dvec3(109, 73, 45)
    ));
    // renderables.emplace("iss", std::make_unique<Cube>(
    //     Material{
    //         .shader = gEng.getShader("entity"),
    //         .uBaseColor = glm::vec4(1, 0, 1, 1),
    //     },
    //     toRenderUnits(colliders["iss"]->getMaxRadius())
    // ));
    renderables.emplace("iss", std::make_unique<Model>(
        "resources/assets/models/iss.glb",
        Material{
            .shader = gEng.getShader("entity"),
            .uAmbientLighting = glm::vec3(0.02f),
        },
        toRenderUnits(colliders["iss"]->getMaxRadius())
    ));
}

Simulation::~Simulation() {}

void Simulation::syncPhysicsUpdate() {
    for (auto& [id, rb] : rigidbodies) {
        if (!renderables.contains(id)) continue;
        std::unique_ptr<Renderable>& rend = renderables.at(id);
        rend->realPos = rb.pos;
        rend->rotation = glm::mat4_cast(rb.rot);
    }
}

void Simulation::clear() {
    renderables.clear();
    rigidbodies.clear();
}
