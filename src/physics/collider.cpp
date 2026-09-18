#include "physics/collider.h"
#include "glm/gtc/type_ptr.hpp"
#include "physics/rigidbody.h"
#include <glm/gtx/quaternion.hpp> 
#include <cstdio>

// MTV computed uses OBB as source, so mtv is how much we need to displace the OBB by to resolve collision
CollisionInfo detectCollisionSphereOBB(SphereCollider* sphere, OBBCollider* obb) {
    glm::dvec3 localSphereCenterPos = glm::inverse(obb->rot) * (sphere->centerPos - obb->centerPos);

    glm::dvec3 closestPointLocal(
        glm::clamp(localSphereCenterPos.x, -obb->halfExtent.x, obb->halfExtent.x),
        glm::clamp(localSphereCenterPos.y, -obb->halfExtent.y, obb->halfExtent.y),
        glm::clamp(localSphereCenterPos.z, -obb->halfExtent.z, obb->halfExtent.z)
    );

    glm::dvec3 closestPointOnOBB = obb->rot * closestPointLocal + obb->centerPos;

    glm::dvec3 cpOffset = sphere->centerPos - closestPointOnOBB;
    double dist2 = glm::length2(cpOffset);

    if (dist2 >= sphere->radius * sphere->radius)
        return { .isColliding = false };

    if (dist2 > 1e-12) {
        double dist = glm::sqrt(dist2);
        glm::dvec3 dir = cpOffset / dist;
        double penetration = sphere->radius - dist;
        glm::dvec3 mtv = -penetration * dir;
        return { 
            .isColliding = glm::length2(mtv) > 1e-12,
            .mtv = mtv,
            .contactPoint1 = closestPointOnOBB,
            .contactPoint2 = closestPointOnOBB + mtv
        };
    }

    glm::dvec3 pen = glm::dvec3(
        obb->halfExtent.x - glm::abs(localSphereCenterPos.x),
        obb->halfExtent.y - glm::abs(localSphereCenterPos.y),
        obb->halfExtent.z - glm::abs(localSphereCenterPos.z)
    );

    glm::dvec3 localDir(0.0);
    double penetration;
    if (pen.x <= pen.y && pen.x <= pen.z) {
        localDir.x = localSphereCenterPos.x < 0 ? -1.0 : 1.0;
        penetration = pen.x + sphere->radius;
    } else if (pen.y <= pen.x && pen.y <= pen.z) {
        localDir.y = localSphereCenterPos.y < 0 ? -1.0 : 1.0;
        penetration = pen.y + sphere->radius;
    } else {
        localDir.z = localSphereCenterPos.z < 0 ? -1.0 : 1.0;
        penetration = pen.z + sphere->radius;
    }
    glm::dvec3 worldDir = obb->rot * localDir;
    glm::dvec3 mtv = -penetration * worldDir;
    return { 
        .isColliding = glm::length2(mtv) > 1e-12,
        .mtv = mtv, 
        .contactPoint1 = closestPointOnOBB,
        .contactPoint2 = closestPointOnOBB + mtv
    };
}

Collider::Collider(const std::string& id, RigidBody* rb, double restitution, double frictionCoeff, glm::dvec3 rbOffset)
    : id(id), centerPos(rb->pos), restitution(restitution), frictionCoeff(frictionCoeff), rbOffset(rbOffset) {
    updateFromRigidBody(rb);
}

void Collider::updateFromRigidBody(RigidBody* rb) {
    centerPos = rb->pos + rbOffset;
    rot = glm::dquat(rb->rot);
}

OBBCollider::OBBCollider(const std::string& id, RigidBody* rb, double restitution, double frictionCoeff, glm::dvec3 fullExtent, glm::dvec3 rbOffset)
    : Collider(id, rb, restitution, frictionCoeff, rbOffset), halfExtent(fullExtent / 2.0) {}

CollisionInfo OBBCollider::detectCollision(Collider* other) {
    if (auto obb = dynamic_cast<OBBCollider*>(other)) {
        // TODO: SAT
        return { .isColliding = false };
    } else if (auto sphere = dynamic_cast<SphereCollider*>(other)) {
        return detectCollisionSphereOBB(sphere, this);
    }
    return { .isColliding = false };
}

const double OBBCollider::getMaxRadius() const {
    return glm::length(halfExtent);
}

glm::dmat3 OBBCollider::computeInertiaTensorBody(double mass) {
    glm::dvec3 dims = 2.0 * halfExtent;
    double Ixx = (1.0/12.0) * mass * (dims.y*dims.y + dims.z*dims.z);
    double Iyy = (1.0/12.0) * mass * (dims.x*dims.x + dims.z*dims.z);
    double Izz = (1.0/12.0) * mass * (dims.x*dims.x + dims.y*dims.y);
    return glm::dmat3(
        Ixx, 0.0, 0.0,
        0.0, Iyy, 0.0,
        0.0, 0.0, Izz
    );
}

SphereCollider::SphereCollider(const std::string& id, RigidBody* rb, double restitution, double frictionCoeff, double radius, glm::dvec3 rbOffset)
    : Collider(id, rb, restitution, frictionCoeff, rbOffset), radius(radius) {}

CollisionInfo SphereCollider::detectCollision(Collider* other) {
    if (auto obb = dynamic_cast<OBBCollider*>(other)) {
        CollisionInfo c = detectCollisionSphereOBB(this, obb);
            return { 
                .isColliding = c.isColliding, 
                .mtv = -c.mtv, 
                .contactPoint1 = c.contactPoint2,
                .contactPoint2 = c.contactPoint1
            };
    } else if (auto sphere = dynamic_cast<SphereCollider*>(other)) {
        double dist2 = glm::distance2(centerPos, sphere->centerPos);
        double sumRadii = radius + sphere->radius;
        if (dist2 >= sumRadii * sumRadii) return { .isColliding = false };
        
    }
    return { .isColliding = false };
}

const double SphereCollider::getMaxRadius() const {
    return radius;
}

glm::dmat3 SphereCollider::computeInertiaTensorBody(double mass) {
    double I = (2.0/5.0) * mass * radius*radius;
    return glm::dmat3(
        I, 0.0, 0.0,
        0.0, I, 0.0,
        0.0, 0.0, I 
    );
}
