#include "hardware/encoder/Rotation.hpp"
#include "pros/error.h"

namespace lemlib {
Rotation::Rotation(int port)
    : rotation(port) {}

Rotation::Rotation(pros::Rotation rotation)
    : rotation(rotation) {}

tl::expected<void, EncoderError> Rotation::calibrate() {
    const auto result = isConnected(); // check if the sensor is connected
    // handle the result
    if (!result.has_value()) return tl::unexpected<EncoderError>(result.error());
    else if (result.value() == false) return tl::unexpected<EncoderError>(EncoderError::NOT_CONNECTED);
    return {}; // return void if there are no errors
}

tl::expected<bool, EncoderError> Rotation::isCalibrated() {
    const auto result = isConnected(); // check if the sensor is connected
    // handle the result
    if (!result.has_value()) return tl::unexpected<EncoderError>(result.error());
    else if (result.value() == false) return tl::unexpected<EncoderError>(EncoderError::NOT_CONNECTED);
    return true; // return true if there are no errors
}

tl::expected<bool, EncoderError> Rotation::isCalibrating() {
    const auto result = isConnected(); // check if the sensor is connected
    // handle the result
    if (!result.has_value()) return tl::unexpected<EncoderError>(result.error());
    else if (result.value() == false) return tl::unexpected<EncoderError>(EncoderError::NOT_CONNECTED);
    return false; // return false if there are no errors
}

tl::expected<bool, EncoderError> Rotation::isConnected() {
    // check if the sensor is connected
    // we don't check for errors because the pros::Rotation::is_installed() function can't return an error value
    return rotation.is_installed();
}

tl::expected<Angle, EncoderError> Rotation::getAbsoluteAngle() {
    const int pos = rotation.get_angle(); // get the angle of the rotation sensor
    if (pos == PROS_ERR) { // check if errors occurred
        // check if the sensor is not connected
        if (errno == ENXIO || errno == ENODEV) return tl::unexpected(EncoderError::NOT_CONNECTED);
        // else return an unknown error
        else return tl::unexpected(EncoderError::UNKNOWN);
    }
    // return the angle of the rotation sensor if there are no errors
    // the angle is multiplied by 0.01 because the pros::Rotation::get_angle() function returns the position in
    // centidegrees
    return from_sDeg(pos * 0.01);
}

tl::expected<Angle, EncoderError> Rotation::getRelativeAngle() {
    const int pos = rotation.get_position(); // get the position of the rotation sensor
    if (pos == PROS_ERR) { // check if errors occurred
        // check if the sensor is not connected
        if (errno == ENXIO || errno == ENODEV) return tl::unexpected(EncoderError::NOT_CONNECTED);
        // else return an unknown error
        else return tl::unexpected(EncoderError::UNKNOWN);
    }
    // return the angle of the rotation sensor if there are no errors
    // the angle is multiplied by 0.01 because the pros::Rotation::get_position() function returns the position in
    // centidegrees
    return from_sDeg(pos * 0.01);
}

tl::expected<void, EncoderError> Rotation::setAbsoluteAngle(Angle) {
    // the V5 Rotation sensor does not support setting the absolute angle
    // directly, as it can only set the current absolute angle to 0
    // so we do just that
    // TODO: follow up with PROS to ensure the reset bug is fixed (https://github.com/purduesigbots/pros/issues/688)
    const auto result = rotation.reset();
    // check for errors
    if (result == PROS_ERR) {
        // check if the sensor is not connected
        if (errno == ENXIO || errno == ENODEV) return tl::unexpected(EncoderError::NOT_CONNECTED);
        // else return an unknown error
        else return tl::unexpected(EncoderError::UNKNOWN);
    }
    // return void if there are no errors
    return {};
}

tl::expected<void, EncoderError> Rotation::setRelativeAngle(Angle angle) {
    const auto result = rotation.set_position(to_sDeg(angle) * 100);
    // check for errors
    if (result == PROS_ERR) {
        // check if the sensor is not connected
        if (errno == ENXIO || errno == ENODEV) return tl::unexpected(EncoderError::NOT_CONNECTED);
        // else return an unknown error
        else return tl::unexpected(EncoderError::UNKNOWN);
    }
    // return void if there are no errors
    return {};
}

tl::expected<void, EncoderError> Rotation::setReversed(bool reversed) {
    const auto result = rotation.set_reversed(reversed);
    // check for errors
    if (result == PROS_ERR) {
        // check if the sensor is not connected
        if (errno == ENXIO || errno == ENODEV) return tl::unexpected(EncoderError::NOT_CONNECTED);
        // else return an unknown error
        else return tl::unexpected(EncoderError::UNKNOWN);
    }
    // return void if there are no errors
    return {};
}

tl::expected<bool, EncoderError> Rotation::isReversed() {
    const auto result = rotation.get_reversed();
    // check for errors
    if (result == PROS_ERR) {
        // check if the sensor is not connected
        if (errno == ENXIO || errno == ENODEV) return tl::unexpected(EncoderError::NOT_CONNECTED);
        // else return an unknown error
        else return tl::unexpected(EncoderError::UNKNOWN);
    }
    // return whether the sensor is reversed or not if there are no errors
    return result;
}

tl::expected<int, EncoderError> Rotation::getPort() {
    // rotation.get_port() can't return an error value
    // so we don't need to check for errors
    return rotation.get_port();
}
} // namespace lemlib