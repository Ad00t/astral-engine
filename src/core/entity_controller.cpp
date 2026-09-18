#include "core/entity_controller.h"
#include <cstdio>
#include <glm/glm.hpp>
#include <glm/gtc/random.hpp>
#include <glm/gtc/quaternion.hpp>

glm::dvec3 vecToBody(const RigidBody& rb, glm::dvec3 global) {
    return glm::inverse(glm::dquat(rb.rot)) * global;
}

glm::dvec3 vecToGlobal(const RigidBody& rb, glm::dvec3 body) {
    return glm::dquat(rb.rot) * body;
}

Thruster::Thruster(Type type, glm::dvec3 offset, glm::dvec3 dir)
    : type(type), offset(offset), dir(dir) {}

void Thruster::applyThrust(RigidBody& rb) {
    rb.applyForceAtPoint(vecToGlobal(rb, thrust * dir), rb.pos + vecToGlobal(rb, offset));    
}

EntityController::EntityController(double sensorNoise)
    : sensorNoise(sensorNoise) {}

StarshipController::StarshipController(double sensorNoise) : EntityController(sensorNoise) {
    thrusters.emplace("main_engine", Thruster(
        Thruster::Type::CHEMICAL,
        glm::dvec3(0.0, -25.0, 0.0), 
        glm::dvec3(0.0, 1.0, 0.0)
    ));
}

void StarshipController::updateSensors(const RigidBody& rb) {
    sensorData.linearAccel = vecToBody(rb, rb.acc - rb.g_acc) + glm::linearRand(glm::dvec3(-sensorNoise), glm::dvec3(sensorNoise));
    sensorData.gyro = vecToBody(rb, rb.ang_vel) + glm::linearRand(glm::dvec3(-sensorNoise), glm::dvec3(sensorNoise));
}

void StarshipController::runControlLoop(RigidBody& rb, double dT) {
    Thruster& me = thrusters.at("main_engine");
    me.thrust = 0.0;

    if (0 < elapsedTime && elapsedTime <= 100.0) {
        me.thrust = 7.44e7;
        me.dir = glm::dvec3(0.0, 1.0, 0.0);
    }

    me.applyThrust(rb);
    printf("T=%.3f -- acc %.3f %.3f %.3f -- gyro %.3f %.3f %.3f -- me %.3f %.3f %.3f %.3f\n", elapsedTime,
            sensorData.linearAccel.x, sensorData.linearAccel.y, sensorData.linearAccel.z,
            sensorData.gyro.x, sensorData.gyro.y, sensorData.gyro.z,
            me.thrust, me.dir.x, me.dir.y, me.dir.z);
    elapsedTime += dT;
}
