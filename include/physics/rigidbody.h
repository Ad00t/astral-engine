#ifndef RIGIDBODY_H
#define RIGIDBODY_H

#include <glm/glm.hpp>
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/quaternion.hpp>

struct OrbitElements {
    double a;       // Semi-major axis (m)
    double e;       // Eccentricity
    double i;       // Inclination (radians)
    double w;       // Argument of perigee (radians)
    double raan;    // Right ascension of ascending node
    double nu;      // True anomaly
};

class RigidBody {
public:
    glm::dvec3 pos = glm::dvec3(0.0);               // m
    glm::dvec3 vel = glm::dvec3(0.0);               // m/s
    glm::dvec3 acc = glm::dvec3(0.0);               // m/s^2
    glm::dvec3 acc_new = glm::dvec3(0.0);           // m/s^2
    glm::dvec3 g_acc = glm::dvec3(0.0);             // m/s^2

    glm::quat rot = glm::quat(1.0, 0.0, 0.0, 0.0);  // quaternion
    glm::dvec3 ang_vel = glm::dvec3(0.0);           // rad/s
    glm::dvec3 ang_acc = glm::dvec3(0.0);           // rad/s^2
    glm::dvec3 ang_acc_new = glm::dvec3(0.0);       // rad/s^2
    
    double mass = 1.0;                              // kg
    // this is set by Collider::create
    glm::dmat3 inertiaTensorBody = glm::dmat3(0.0);     // kgm^2, body frame

    RigidBody(glm::dvec3 pos, glm::dvec3 vel, glm::quat rot, glm::dvec3 ang_vel, double mass);
    RigidBody(const RigidBody& central, OrbitElements oe, glm::quat rot, glm::dvec3 ang_vel, double mass);
    RigidBody() = default;
    ~RigidBody() = default;

    void applyForceAtPoint(const glm::dvec3& force, const glm::dvec3& point, bool isGravity = false);
    glm::dmat3 getInvInertiaWorld() const;

    void fromOrbitElements(const RigidBody& central, OrbitElements& oe);
    void toOrbitElements(const RigidBody& central, OrbitElements& oe);
};

#endif
