#ifndef COLLIDER_H
#define COLLIDER_H

#include "physics/rigidbody.h"
#include <glm/glm.hpp>
#include <vector>
#include <memory>

struct CollisionInfo {
    bool isColliding = false;
    glm::dvec3 mtv = glm::dvec3(0.0);
    glm::dvec3 contactPoint1 = glm::dvec3(0.0);
    glm::dvec3 contactPoint2 = glm::dvec3(0.0);
};

class Collider {
public:
    glm::dvec3 centerPos;
    glm::dquat rot;
    double restitution;
    double frictionCoeff;

    Collider(RigidBody& rb, double restitution, double frictionCoeff);
    virtual ~Collider() = default;

    template <typename T, typename... Args>
    static std::unique_ptr<T> create(RigidBody& rb, Args&&... args) {
        auto obj = std::make_unique<T>(rb, std::forward<Args>(args)...);
        rb.inertiaTensorBody = obj->computeInertiaTensorBody(rb.mass);
        return obj;
    }

    void updateFromRigidBody(const RigidBody& rb);
    // Should implement dependent on other Collider's derived type / geometry
    virtual CollisionInfo detectCollision(Collider* other) = 0; 
    virtual const double getMaxRadius() const = 0;
    virtual glm::dmat3 computeInertiaTensorBody(double mass) = 0;
};
   
// Oriented bounding box
class OBBCollider : public Collider {
public:
    glm::dvec3 halfExtent; // Positive offset of rect3d corner points in local frame

    OBBCollider(RigidBody& rb, double restitution, double frictionCoeff, glm::dvec3 fullExtent);

    virtual CollisionInfo detectCollision(Collider* other) override; 
    const double getMaxRadius() const override;
    glm::dmat3 computeInertiaTensorBody(double mass) override;
};

class SphereCollider : public Collider {
public:
    double radius;

    SphereCollider(RigidBody& rb, double restitution, double frictionCoeff, double radius);

    virtual CollisionInfo detectCollision(Collider* other) override; 
    const double getMaxRadius() const override;
    glm::dmat3 computeInertiaTensorBody(double mass) override;
};

// class CompoundCollider : public Collider {
// public:
//     std::vector<std::unique_ptr<Collider>> subColliders;
//
//     CompoundCollider(RigidBody& rb, std::vector<std::unique_ptr<Collider>> subColliders);
//
//     void updateFromRigidBody(const RigidBody& rb) override;
//     CollisionInfo detectCollision(Collider* other) override; 
//     const double getMaxRadius() const override;
//     virtual glm::dmat3 computeInertiaTensorBody(double mass) override;
// };

#endif
