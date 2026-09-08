#ifndef COLLIDER_H
#define COLLIDER_H

#include "physics/rigidbody.h"
#include <glm/glm.hpp>

struct CollisionInfo {
    bool isColliding = false;
    glm::dvec3 mtv = glm::dvec3(0.0);
    glm::dvec3 contactPoint1 = glm::dvec3(0.0);
    glm::dvec3 contactPoint2 = glm::dvec3(0.0);
};

class Collider {
public:
    glm::dvec3 centerPos;
    double restitution;
    double frictionCoeff;

    Collider(const RigidBody& rb, double restitution, double frictionCoeff);
    virtual ~Collider() = default;

    virtual void updateFromRigidBody(const RigidBody& rb);
    // Should implement dependent on other Collider's derived type / geometry
    virtual CollisionInfo detectCollision(Collider* other) = 0; 
    virtual const double getMaxRadius() const = 0;
};
   
// Oriented bounding box
class OBBCollider : public Collider {
public:
    glm::dvec3 halfExtent; // Positive offset of rect3d corner points in local frame
    glm::dquat rot;

    OBBCollider(const RigidBody& rb, double restitution, double frictionCoeff, glm::dvec3 fullExtent);

    void updateFromRigidBody(const RigidBody& rb) override;
    virtual CollisionInfo detectCollision(Collider* other) override; 
    const double getMaxRadius() const override;
};

class SphereCollider : public Collider {
public:
    double radius;

    SphereCollider(const RigidBody& rb, double restitution, double frictionCoeff, double radius);

    void updateFromRigidBody(const RigidBody& rb) override;
    virtual CollisionInfo detectCollision(Collider* other) override; 
    const double getMaxRadius() const override;
};

#endif
