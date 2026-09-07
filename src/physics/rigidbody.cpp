#include "physics/rigidbody.h"
#include <glm/glm.hpp>

RigidBody::RigidBody(glm::dvec3 pos, glm::dvec3 vel, glm::quat rot, glm::dvec3 ang_vel,
                 double radius, double mass)
    : pos(pos), vel(vel), acc(glm::dvec3(0.0)), acc_new(glm::dvec3()), 
      rot(rot), ang_vel(ang_vel),
      radius(radius), mass(mass) {}
