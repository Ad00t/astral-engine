#ifndef COLLIDER_H
#define COLLIDER_H

#include "physics/rigidbody.h"
#include <glm/glm.hpp>

class Collider {
public:
    glm::dvec3 centerPos;
    double restitution;

    Collider(const RigidBody& rb, double restitution);
    virtual ~Collider() = default;

    virtual void updateFromRigidBody(const RigidBody& rb);
    // Should implement dependent on other Collider's derived type / geometry
    virtual glm::dvec3 computeMTV(Collider* other) = 0; 
    virtual const double getMaxRadius() const = 0;
};
   
// Oriented bounding box
class OBBCollider : public Collider {
public:
    glm::dvec3 halfExtent; // Positive offset of rect3d corner points in local frame
    glm::dquat rot;

    OBBCollider(const RigidBody& rb, double restitution, glm::dvec3 fullExtent);

    void updateFromRigidBody(const RigidBody& rb) override;
    glm::dvec3 computeMTV(Collider* other) override; 
    const double getMaxRadius() const override;
};

class SphereCollider : public Collider {
public:
    double radius;

    SphereCollider(const RigidBody& rb, double restitution, double radius);

    void updateFromRigidBody(const RigidBody& rb) override;
    glm::dvec3 computeMTV(Collider* other) override; 
    const double getMaxRadius() const override;
};

#endif
