#include "core/simulation.h"
#include "core/controller.h"
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

Entity::Entity(const std::string& id) : id(id) {}

void Entity::addRenderable(std::unique_ptr<Renderable> rend) {
    renderables.emplace(rend->id, std::move(rend));
}

void Entity::addCollider(std::unique_ptr<Collider> coll) {
    colliders.emplace(coll->id, std::move(coll));
}

Simulation::Simulation(GraphicsEngine& gEng, PhysicsEngine& pEng) {
    runPhysics.store(true);

    // Spacebox
    std::unique_ptr<Entity> spacebox = std::make_unique<Entity>("spacebox");
    spacebox->addRenderable(std::make_unique<SkyBox>("spacebox",
        Material{
            .shader = gEng.getShader("skybox"), 
            .textureID = gEng.getTextureID("cubemap/spacebox"),
            .uBaseColor = glm::vec4(0, 0, 0, 1) 
        }
    ));

    // Sun
    std::unique_ptr<Entity> sun = std::make_unique<Entity>("sun");
    sun->rigidbody = std::make_unique<RigidBody>(
        glm::dvec3(0, 0, 0), 
        glm::dvec3(0, 0, 0), 
        glm::quat(0.568933, 0.059228, 0.305817, 0.761107),
        glm::dvec3(3.505831e-7, -8.893373e-8, 2.842410e-6),
        M_SUN 
    );
    sun->addCollider(Collider::create<SphereCollider>(
        "sun", sun->rigidbody.get(),
        0.0, 1.0,
        R_SUN 
    ));
    sun->addRenderable(std::make_unique<CelestialBody>(
        "sun", Material{
            .shader = gEng.getShader("entity"), 
            .textureID = gEng.getTextureID("uvmap/sun"),
            .uBaseColor = glm::vec4(1, 1, 0, 1),
            .uUseTexture = true,
            .uEmissiveLighting = glm::vec3(1000.0f),
            .uUseRaytracedSphere = true
        }
    ));

    // Earth
    std::unique_ptr<Entity> earth = std::make_unique<Entity>("earth");
    earth->rigidbody = std::make_unique<RigidBody>(
        glm::dvec3(1.496e11, 0, 0),
        glm::dvec3(0, 3.0e4, 0), 
        glm::quat(0.750882, -0.155769, 0.130365, 0.628425),
        glm::dvec3(4.47e-21, 2.900637e-5, 6.690385e-5),
        M_EARTH
    );
    earth->addCollider(Collider::create<SphereCollider>(
        "earth", earth->rigidbody.get(),
        0.0, 0.75,
        R_EARTH
    ));
    earth->addRenderable(std::make_unique<CelestialBody>(
        "earth", Material{
            .shader = gEng.getShader("entity"), 
            .textureID = gEng.getTextureID("uvmap/earth_day"),
            .textureID2 = gEng.getTextureID("uvmap/earth_night"),
            .uBaseColor = glm::vec4(0, 0, 1, 1), 
            .uUseTexture = true,
            .uUseDayNightBlend = true,
            .uUseRaytracedSphere = true,
            .atmosphere = AtmosphereParams{ .enabled = true }
        }
    ));
   
    // Moon
    std::unique_ptr<Entity> moon = std::make_unique<Entity>("moon");
    moon->rigidbody = std::make_unique<RigidBody>(
        glm::dvec3(1.496e11 + 3.84e8, 0, 0),
        glm::dvec3(0, 3.0e4 + 1.022e3, 0), 
        glm::quat(0.328257, 0.000045, 0.375904, 0.866570),
        glm::dvec3(-9.432401e-11, -9.992006e-10, 2.661699e-6),
        M_MOON 
    );
    moon->addCollider(Collider::create<SphereCollider>(
        "moon", moon->rigidbody.get(),
        0.0, 1.0,
        R_MOON 
    ));
    moon->addRenderable(std::make_unique<CelestialBody>(
        "moon", Material{
            .shader = gEng.getShader("entity"),
            .textureID = gEng.getTextureID("uvmap/moon"),
            .uBaseColor = glm::vec4(1, 1, 1, 1),
            .uUseTexture = true,
            .uUseRaytracedSphere = true
        }
    ));

    // ISS
    std::unique_ptr<Entity> iss = std::make_unique<Entity>("iss");
    iss->rigidbody = std::make_unique<RigidBody>(
        earth->rigidbody.get(),
        OrbitElements{
            .a = 6.7975e6,
            .e = 0.0004984,
            .i = glm::radians(51.6306),
            .w = glm::radians(115.8922),
            .raan = glm::radians(252.7093),
            .nu = glm::radians(0.1)
        },
        glm::quat(1, 0, 0, 0),
        glm::dvec3(0, 0, 0),
        450000
    );
    iss->addCollider(Collider::create<OBBCollider>(
        "iss:hull", iss->rigidbody.get(),
        0.0, 1.0,
        glm::dvec3(109, 73, 45)
    ));
    iss->addRenderable(std::make_unique<Model>(
        "iss:hull", "resources/assets/models/iss.glb",
        Material{
            .shader = gEng.getShader("entity"),
        },
        glm::vec3(1.5f, 1.5f, 1.3f)
    ));

    // Starship 
    std::unique_ptr<Entity> starship = std::make_unique<Entity>("starship");
    starship->rigidbody = std::make_unique<RigidBody>(
        glm::dvec3(1.496e11 - (R_EARTH + 30), 0, 0),
        glm::dvec3(0, 3.0e4 - 436, 0), 
        glm::angleAxis(glm::radians(90.0f), glm::vec3(0.0f, 0.0f, 1.0f)),
        glm::dvec3(0, 0, 0),
        5300000 
    );
    starship->controller = std::make_unique<StarshipController>(
        1e-3
    );
    starship->addCollider(Collider::create<OBBCollider>(
        "starship:hull", starship->rigidbody.get(),
        0.0, 0.75,
        glm::dvec3(9, 51, 9)
    ));
    starship->addRenderable(std::make_unique<Model>(
        "starship:hull", "resources/assets/models/starship.glb",
        Material{
            .shader = gEng.getShader("entity")
        },
        glm::vec3(1.0f, 2.0f, 1.0f)
    ));
    starship->addRenderable(std::make_unique<ExhaustPlume>(
        "starship:main_engine", Material{
            .shader = gEng.getShader("exhaust"),
            .uBaseColor = glm::vec4(1, 1, 1, 1)
        },
        glm::vec3(1.0f), 
        starship->controller->thrusters.at("starship:main_engine").get()
    ));

    addEntity(std::move(spacebox));
    addEntity(std::move(sun));
    addEntity(std::move(earth));
    addEntity(std::move(moon));
    addEntity(std::move(iss));
    addEntity(std::move(starship));
}

void Simulation::addEntity(std::unique_ptr<Entity> entity) {
    entities.emplace(entity->id, std::move(entity));
}

void Simulation::updateFromPhysics() {
    for (auto& [entity_id, entity] : entities) {
        if (entity->rigidbody == nullptr) continue;
        for (auto& [component_id, coll] : entity->colliders) {
            coll->updateFromRigidBody(entity->rigidbody.get());
        }
        for (auto& [component_id, rend] : entity->renderables) {
            rend->updateFromRigidBody(entity->rigidbody.get());
        }
    }
}
