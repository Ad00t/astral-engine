#include "simulation.h"
#include "graphics/camera.h"
#include "graphics/graphics_engine.h"
#include "graphics/renderable.h"        
#include "physics/physics_engine.h"
#include "utils.h"
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/quaternion.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <memory>

const double R_SUN = 7.957e8;
const double M_SUN = 1.989e30;

const double R_EARTH = 6.371e6;
const double M_EARTH = 5.972e24;

const double R_MOON = 1.7375e6;
const double M_MOON = 7.35e22;

Simulation::Simulation(GraphicsEngine& gEng, PhysicsEngine& pEng) {
    runPhysics.store(true);

    // Spacebox
    renderables.emplace("spacebox", std::make_unique<SkyBox>(
        Material(
            glm::vec3(0.0), 
            gEng.getShader("skybox"), 
            gEng.getTextureID("cubemap/spacebox")
        )
    ));

    // Sun
    renderables.emplace("sun", std::make_unique<Sphere>(
        Material(
            glm::vec3(1, 1, 0), 
            gEng.getShader("simobj"), 
            gEng.getTextureID("uvmap/sun")
        ),
        toRender(R_SUN)
    ));
    rigidbodies.emplace("sun", RigidBody(
        glm::dvec3(0, 0, 0), 
        glm::dvec3(0, 0, 0), 
        glm::quat(0.568933,  0.059228,  0.305817,  0.761107),
        glm::dvec3(3.505831e-7, -8.893373e-8, 2.842410e-6),
        R_SUN, M_SUN 
    ));

    // Earth
    renderables.emplace("earth", std::make_unique<Sphere>(
        Material(
            glm::vec3(0, 0, 1), 
            gEng.getShader("simobj"), 
            gEng.getTextureID("uvmap/earth_day")
        ),
        toRender(R_EARTH)
    ));
    rigidbodies.emplace("earth", RigidBody(
        glm::dvec3(1.496e11, 0, 0),
        glm::dvec3(0, 3.0e4, 0), 
        glm::quat(0.750882, -0.155769, 0.130365, 0.628425),
        glm::dvec3(4.47e-21, 2.900637e-5, 6.690385e-5),
        R_EARTH, M_EARTH
    ));
   
    // Moon
    renderables.emplace("moon", std::make_unique<Sphere>(
        Material(
            glm::vec3(1, 1, 1), 
            gEng.getShader("simobj"),
            gEng.getTextureID("uvmap/moon")
        ),
        toRender(R_MOON)
    ));
    rigidbodies.emplace("moon", RigidBody(
        glm::dvec3(1.496e11 + 3.84e8, 0, 0),
        glm::dvec3(0, 3.0e4 + 1.022e3, 0), 
        glm::quat(0.328257,  0.000045, 0.375904, 0.866570),
        glm::dvec3(-9.432401e-11, -9.992006e-10, 2.661699e-6),
        R_MOON, M_MOON 
    ));

    pEng.initialize(rigidbodies);
    gEng.cam->setTarget(renderables.at("earth").get());
}

Simulation::~Simulation() {}

void Simulation::syncPhysicsToRender() {
    for (auto& [id, rb] : rigidbodies) {
        if (!renderables.contains(id)) continue;
        glm::mat4 model = glm::mat4(1.0f);
        model = glm::translate(model, toRender(rb.pos));
        glm::mat4 rotationMatrix = glm::mat4_cast(rb.rot);
        model = model * rotationMatrix; 
        renderables.at(id)->setModel(model);
    }
}

void Simulation::clear() {
    renderables.clear();
    rigidbodies.clear();
}
