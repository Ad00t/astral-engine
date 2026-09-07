#include "physics/collider.h"
#include "glm/gtc/type_ptr.hpp"
#include "physics/rigidbody.h"
#include <glm/gtx/quaternion.hpp> 
#include <cstdio>

// MTV computed uses OBB as source, so mtv is how much we need to displace the OBB by to resolve collision
glm::dvec3 computeMTVSphereOBB(SphereCollider* sphere, OBBCollider* obb) {
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
        return glm::dvec3(0.0);

    if (dist2 > 1e-12) {
        double dist = glm::sqrt(dist2);
        glm::dvec3 dir = cpOffset / dist;
        double penetration = sphere->radius - dist;
        return -penetration * dir;
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
    return -penetration * worldDir;
}

Collider::Collider(const RigidBody& rb, double restitution)
    : centerPos(rb.pos), restitution(restitution) {
    updateFromRigidBody(rb);
}

void Collider::updateFromRigidBody(const RigidBody& rb) {
    centerPos = rb.pos;
}

OBBCollider::OBBCollider(const RigidBody& rb, double restitution, glm::dvec3 halfExtent)
    : Collider(rb, restitution), halfExtent(halfExtent) {}

void OBBCollider::updateFromRigidBody(const RigidBody& rb) {
    Collider::updateFromRigidBody(rb);
    rot = glm::dquat(rb.rot);
}

glm::dvec3 OBBCollider::computeMTV(Collider* other) {
    if (auto obb = dynamic_cast<OBBCollider*>(other)) {
        // TODO: SAT
        return glm::dvec3(0.0);
    } else if (auto sphere = dynamic_cast<SphereCollider*>(other)) {
        return computeMTVSphereOBB(sphere, this);
    }
    return glm::dvec3(0.0);
}

SphereCollider::SphereCollider(const RigidBody& rb, double restitution, double radius)
    : Collider(rb, restitution), radius(radius) {}

void SphereCollider::updateFromRigidBody(const RigidBody& rb) {
    Collider::updateFromRigidBody(rb);
}

glm::dvec3 SphereCollider::computeMTV(Collider* other) {
    if (auto obb = dynamic_cast<OBBCollider*>(other)) {
        return -computeMTVSphereOBB(this, obb); // Negate so sphere is source
    } else if (auto sphere = dynamic_cast<SphereCollider*>(other)) {
        double dist2 = glm::distance2(centerPos, sphere->centerPos);
        double sumRadii = radius + sphere->radius;
        if (dist2 >= sumRadii * sumRadii) return glm::dvec3(0.0);
        
    }
    return glm::dvec3(0.0);
}
