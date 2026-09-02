#ifndef RIGIDBODY_H
#define RIGIDBODY_H

#include <glm/glm.hpp>
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/quaternion.hpp>

class RigidBody {
public:
    glm::dvec3 pos;         // m
    glm::dvec3 vel;         // m/s
    glm::dvec3 acc;         // m/s^2
    glm::dvec3 acc_new;     // m/s^2

    glm::quat rot;          // quaternion
    glm::dvec3 ang_vel;     // rad/s
    
    // Eventually want to replace this with a bounding box / 
    // inertial mass distribution, unified with the Renderable component.
    // This will be important for accurate torque modeling
    double radius;          // m
    double mass;            // kg

    RigidBody(glm::dvec3 pos = glm::dvec3(0.0),
            glm::dvec3 vel = glm::dvec3(0.0),
            glm::quat rot = glm::quat(1.0, 0.0, 0.0, 0.0),
            glm::dvec3 ang_vel = glm::dvec3(0.0),
            double radius = 1.0, 
            double mass = 1.0);

    virtual ~RigidBody() = default;

    void applyForce(const glm::dvec3& force);
    void integratePos(double dT);
    void integrateVel(double dT);
    void integrateRot(double dT);
};

#endif
