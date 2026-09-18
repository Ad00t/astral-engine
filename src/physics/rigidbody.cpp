#include "physics/rigidbody.h"
#include "utils.h"
#include <glm/glm.hpp>
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/quaternion.hpp>

RigidBody::RigidBody(glm::dvec3 pos, glm::dvec3 vel, glm::quat rot, glm::dvec3 ang_vel, double mass)
    : pos(pos), vel(vel), rot(rot), ang_vel(ang_vel), mass(mass) {}

RigidBody::RigidBody(const RigidBody& central, OrbitElements oe, glm::quat rot, glm::dvec3 ang_vel, double mass)
    : rot(rot), ang_vel(ang_vel), mass(mass) {
    fromOrbitElements(central, oe); 
}

void RigidBody::applyForceAtPoint(const glm::dvec3& force, const glm::dvec3& point, bool isGravity) {
    acc_new += force / mass;
    ang_acc_new += getInvInertiaWorld() * glm::cross(point - pos, force);
    if (isGravity) g_acc += force / mass;
}

glm::dmat3 RigidBody::getInvInertiaWorld() const {
    glm::dmat3 R = glm::toMat3(rot);
    return R * glm::inverse(inertiaTensorBody) * glm::transpose(R);
}

void RigidBody::fromOrbitElements(const RigidBody& central, OrbitElements& oe) {
    double p = oe.a * (1 - oe.e*oe.e);
    double r = p / (1 + oe.e * cos(oe.nu));

    glm::dvec3 r_pqw = glm::dvec3(r * cos(oe.nu), r * sin(oe.nu), 0);
    glm::dvec3 v_pqw = sqrt((G * central.mass) / p) * glm::dvec3(-sin(oe.nu), oe.e + cos(oe.nu), 0);

    glm::quat r_raan = glm::angleAxis(-oe.raan, glm::dvec3(0, 0, 1));
    glm::quat r_i = glm::angleAxis(-oe.i, glm::dvec3(1, 0, 0));
    glm::quat r_w = glm::angleAxis(-oe.w, glm::dvec3(0, 0, 1));
    glm::mat3 R_pqw_to_eci = glm::mat3_cast(r_raan * r_i * r_w);

    glm::dvec3 r_eci = R_pqw_to_eci * r_pqw;
    glm::dvec3 v_eci = R_pqw_to_eci * v_pqw;

    pos = central.pos + r_eci;
    vel = central.vel + v_eci;
}

void RigidBody::toOrbitElements(const RigidBody& central, OrbitElements& oe) {
    // TODO
}


