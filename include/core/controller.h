#ifndef CONTROLLER_H
#define CONTROLLER_H

#include <physics/rigidbody.h>
#include <unordered_map>
#include <string>
#include <memory>

class Thruster {
public:
    enum Type {
        CHEMICAL,
        ELECTRIC,
        COLD_GAS
    };
    Type type;

    glm::dvec3 offset = glm::dvec3(0.0);
    glm::dvec3 dir = glm::dvec3(0.0);
    double thrust = 0.0;

    Thruster(Type type, glm::dvec3 offset, glm::dvec3 dir);
    ~Thruster() = default;

    void applyThrust(RigidBody* rb);
};

class Controller {
protected:
    double sensorNoise = 0;
    double elapsedTime = 0;

    struct SensorData {
        double temperature = 0;
        double pressure = 0;
        double humidity = 0;
        glm::dvec3 linearAccel = glm::dvec3(0.0);
        glm::dvec3 mag = glm::dvec3(0.0);
        glm::dvec3 gyro = glm::dvec3(0.0);
    };
    SensorData sensorData;

public:
    std::unordered_map<std::string, std::unique_ptr<Thruster>> thrusters;

    Controller(double sensorNoise);
    virtual ~Controller() = default;

    virtual void updateSensors(RigidBody* rb) = 0;
    virtual void runControlLoop(RigidBody* rb, double dT) = 0;
};

class StarshipController : public Controller {
public:
    StarshipController(double sensorNoise);
    
    void updateSensors(RigidBody* rb) override;
    void runControlLoop(RigidBody* rb, double dT) override;
};

#endif
