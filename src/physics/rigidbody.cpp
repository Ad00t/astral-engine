#include "physics/rigidbody.h"
#include <glm/glm.hpp>

RigidBody::RigidBody(glm::dvec3 pos, glm::dvec3 vel, glm::quat rot, glm::dvec3 ang_vel,
                 double radius, double mass)
    : pos(pos), vel(vel), acc(glm::dvec3(0.0)), acc_new(glm::dvec3()), 
      rot(rot), ang_vel(ang_vel),
      radius(radius), mass(mass) {}

void RigidBody::applyForce(const glm::dvec3& force) {
    if (mass > 0.0) {
        acc_new += force / mass;
    }
}

void RigidBody::integratePos(double dT) {
    pos = pos + vel * dT + 0.5 * acc * dT * dT;
}

void RigidBody::integrateVel(double dT) {
    vel = vel + 0.5 * (acc + acc_new) * dT;
    acc = acc_new;
    acc_new = glm::dvec3(0.0);
}

void RigidBody::integrateRot(double dT) {
    double ang_vel_norm = glm::l2Norm(ang_vel);
    double theta = ang_vel_norm * dT;
    glm::dvec3 u_hat = ang_vel / ang_vel_norm;

    glm::quat dq(1.0, 0.0, 0.0, 0.0);
    if (theta > 0) {
        dq = glm::quat(
            glm::cos(theta/2), 
            u_hat.x * glm::sin(theta/2),
            u_hat.y * glm::sin(theta/2),
            u_hat.z * glm::sin(theta/2)
        );
    }
    rot = glm::normalize(dq * rot);
}
