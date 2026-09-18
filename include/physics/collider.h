#ifndef COLLIDER_H
#define COLLIDER_H

#include "physics/rigidbody.h"
#include <glm/glm.hpp>
#include <memory>

struct CollisionInfo {
    bool isColliding = false;
    glm::dvec3 mtv = glm::dvec3(0.0);
    glm::dvec3 contactPoint1 = glm::dvec3(0.0);
    glm::dvec3 contactPoint2 = glm::dvec3(0.0);
};

class Collider {
public:
    std::string id;
    glm::dvec3 centerPos;
    glm::dvec3 rbOffset;
    glm::dquat rot;
    double restitution;
    double frictionCoeff;

    Collider(const std::string& id, RigidBody* rb, double restitution, double frictionCoeff, glm::dvec3 rbOffset = glm::dvec3(0.0));
    virtual ~Collider() = default;

    template <typename T, typename... Args>
    static std::unique_ptr<T> create(const std::string& id, RigidBody* rb, Args&&... args) {
        auto obj = std::make_unique<T>(id, rb, std::forward<Args>(args)...);
        rb->inertiaTensorBody = obj->computeInertiaTensorBody(rb->mass);
        return obj;
    }

    void updateFromRigidBody(RigidBody* rb);
    // Should implement dependent on other Collider's derived type / geometry
    virtual CollisionInfo detectCollision(Collider* other) = 0; 
    virtual const double getMaxRadius() const = 0;
    virtual glm::dmat3 computeInertiaTensorBody(double mass) = 0;
};
   
// Oriented bounding box
class OBBCollider : public Collider {
public:
    glm::dvec3 halfExtent; // Positive offset of rect3d corner points in local frame

    OBBCollider(const std::string& id, RigidBody* rb, double restitution, double frictionCoeff, glm::dvec3 fullExtent, glm::dvec3 rbOffset = glm::dvec3(0.0));

    virtual CollisionInfo detectCollision(Collider* other) override; 
    const double getMaxRadius() const override;
    glm::dmat3 computeInertiaTensorBody(double mass) override;
};

class SphereCollider : public Collider {
public:
    double radius;

    SphereCollider(const std::string& id, RigidBody* rb, double restitution, double frictionCoeff, double radius, glm::dvec3 rbOffset = glm::dvec3(0.0));

    virtual CollisionInfo detectCollision(Collider* other) override; 
    const double getMaxRadius() const override;
    glm::dmat3 computeInertiaTensorBody(double mass) override;
};

#endif
