#include "hardware/Motors/MotorGroup.hpp"
#include <climits>

namespace lemlib {
MotorGroup::MotorGroup(std::initializer_list<pros::Motor> motors, AngularVelocity outputVelocity)
    : m_outputVelocity(outputVelocity) {
    for (const pros::Motor& motor : motors) m_ports.push_back(motor.get_port());
}

MotorGroup::MotorGroup(pros::v5::MotorGroup motors, AngularVelocity outputVelocity)
    : m_outputVelocity(outputVelocity),
      m_ports(motors.get_port_all()) {}

int MotorGroup::move(double percent) {
    const std::vector<Motor> motors = getMotors();
    bool success = false;
    for (Motor motor : motors) {
        const int result = motor.move(percent);
        if (result == 0) success = true;
    }
    // as long as one motor moves successfully, return 0 (success)
    return success ? 0 : INT_MAX;
}

int MotorGroup::moveVelocity(AngularVelocity velocity) {
    const std::vector<Motor> motors = getMotors();
    bool success = false;
    for (Motor motor : motors) {
        // since the motors in the group are geared together, we need to account for different gearings
        // of different motors in the group
        const Cartridge cartridge = motor.getCartridge();
        // check for errors
        if (cartridge == Cartridge::INVALID) continue;
        // calculate gear ratio
        const Number ratio = from_rpm(static_cast<int>(cartridge)) / m_outputVelocity;
        const int result = motor.moveVelocity(velocity * ratio);
        if (result == 0) success = true;
    }
    // as long as one motor moves successfully, return 0 (success)
    return success ? 0 : INT_MAX;
}

int MotorGroup::brake() {
    const std::vector<Motor> motors = getMotors();
    bool success = false;
    for (Motor motor : motors) {
        const int result = motor.brake();
        if (result == 0) success = true;
    }
    // as long as one motor brakes successfully, return 0 (success)
    return success ? 0 : INT_MAX;
}

int MotorGroup::setBrakeMode(BrakeMode mode) {
    const std::vector<Motor> motors = getMotors();
    bool success = false;
    for (Motor motor : motors) {
        const int result = motor.setBrakeMode(mode);
        if (result == 0) success = true;
    }
    // as long as one motor sets the brake mode successfully, return 0 (success)
    return success ? 0 : INT_MAX;
}

std::vector<BrakeMode> MotorGroup::getBrakeModes() {
    const std::vector<Motor> motors = getMotors();
    std::vector<BrakeMode> brakeModes;
    for (const Motor& motor : motors) brakeModes.push_back(motor.getBrakeMode());
    return brakeModes;
}

int MotorGroup::isConnected() {
    const std::vector<Motor> motors = getMotors();
    for (Motor motor : motors) {
        const int result = motor.isConnected();
        if (result == 1) return true;
    }
    // if no motors are connected, return 0
    return 0;
}

Angle MotorGroup::getAngle() {
    const std::vector<Motor> motors = getMotors();
    // get the average angle of all motors in the group
    Angle angle = 0_stDeg;
    bool success = false;
    for (Motor motor : motors) {
        // get angle
        const Angle result = motor.getAngle();
        if (result == from_stDeg(INFINITY)) continue; // check for errors
        // get motor cartridge
        const Cartridge cartridge = motor.getCartridge();
        if (cartridge == Cartridge::INVALID) continue; // check for errors
        success = true;
        // calculate the gear ratio
        const Number ratio = m_outputVelocity / from_rpm(static_cast<int>(cartridge));
        std::cout << to_stDeg(result) << std::endl;
        angle += result * ratio;
    }
    // if no motors are connected, return INFINITY
    if (!success) return from_stDeg(INFINITY);
    // otherwise, return the average angle
    return angle / getSize();
}

int MotorGroup::setAngle(Angle angle) {
    const std::vector<Motor> motors = getMotors();
    bool success = false;
    for (Motor motor : motors) {
        // since the motors in the group are geared together, we need to account for different gearings
        // of different motors in the group
        const Cartridge cartridge = motor.getCartridge();
        // check for errors
        if (cartridge == Cartridge::INVALID) continue;
        // calculate gear ratio
        const Number ratio = from_rpm(static_cast<int>(cartridge)) / m_outputVelocity;
        const int result = motor.setAngle(angle * ratio);
        if (result == 0) success = true;
    }
    // as long as one motor sets the angle successfully, return 0 (success)
    return success ? 0 : INT_MAX;
}

int MotorGroup::getSize() const {
    const std::vector<Motor> motors = getMotors();
    int size = 0;
    for (Motor motor : motors)
        if (motor.isConnected()) size++;
    return size;
}

int MotorGroup::addMotor(int port) {
    const std::vector<Motor> motors = getMotors();
    Motor motor = pros::Motor(port);
    // set the motor's brake mode to whatever the first working motor's brake mode is
    for (Motor m : motors) {
        const BrakeMode mode = m.getBrakeMode();
        if (mode == BrakeMode::INVALID) continue;
        if (motor.setBrakeMode(mode)) break;
        else return INT_MAX;
    }
    // calculate the gear ratio
    const Cartridge cartridge = motor.getCartridge();
    if (cartridge == Cartridge::INVALID) return INT_MAX; // check for errors
    const Number ratio = from_rpm(static_cast<int>(cartridge)) / m_outputVelocity;
    const Angle angle = getAngle() * ratio;
    if (angle == from_stDeg(INFINITY)) return INT_MAX; // check for errors
    // set the angle of the new motor
    const int result = motor.setAngle(angle);
    if (result == INT_MAX) return INT_MAX; // check for errors
    m_ports.push_back(port);
    return 0;
}

int MotorGroup::addMotor(Motor motor) { return addMotor(motor.getPort()); }

int MotorGroup::addMotor(Motor motor, bool reversed) {
    // set the motor reversal
    motor.setReversed(reversed);
    return addMotor(motor);
}

void MotorGroup::removeMotor(int port) {
    // remove the motor with the specified port
    auto it = m_ports.begin();
    while (it < m_ports.end()) {
        if (std::abs(*it) == std::abs(port)) {
            m_ports.erase(it);
        } else {
            it++;
        }
    }
}

const std::vector<Motor> MotorGroup::getMotors() const {
    std::vector<Motor> motors;
    for (const int port : m_ports) motors.push_back(pros::Motor(port));
    return motors;
}

void MotorGroup::removeMotor(Motor motor) { removeMotor(motor.getPort()); }
}; // namespace lemlib