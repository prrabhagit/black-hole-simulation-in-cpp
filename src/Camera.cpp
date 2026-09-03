#include "Camera.hpp"

#include <GLFW/glfw3.h>
#include <algorithm>
#include <cmath>

namespace bh::scene {

Camera::Camera(float aspect) : aspect_(aspect) {
    rebuildBasisFromYawPitch(orbitYaw_, orbitPitch_);
}

void Camera::setMode(CameraMode mode) {
    if (mode_ == mode) return;
    mode_ = mode;
    firstMouse_ = true;
}

void Camera::toggleMode() {
    setMode(mode_ == CameraMode::Orbit ? CameraMode::FreeFly : CameraMode::Orbit);
}

void Camera::setOrbitRadius(float r) noexcept {
    orbitRadius_ = std::clamp(r, minRadius_, maxRadius_);
}

void Camera::update(GLFWwindow* window, float dt) {
    if (glfwGetKey(window, GLFW_KEY_1) == GLFW_PRESS) setMode(CameraMode::Orbit);
    if (glfwGetKey(window, GLFW_KEY_2) == GLFW_PRESS) setMode(CameraMode::FreeFly);

    if (mode_ == CameraMode::Orbit) updateOrbit(window, dt);
    else                            updateFreeFly(window, dt);

    pendingScrollDelta_ = 0.0f;
}

void Camera::rebuildBasisFromYawPitch(float yawDeg, float pitchDeg) {
    const float yaw   = glm::radians(yawDeg);
    const float pitch = glm::radians(pitchDeg);

    forward_ = glm::normalize(glm::vec3(
        std::cos(pitch) * std::cos(yaw),
        std::sin(pitch),
        std::cos(pitch) * std::sin(yaw)
    ));
    right_ = glm::normalize(glm::cross(forward_, glm::vec3(0.0f, 1.0f, 0.0f)));
    up_    = glm::normalize(glm::cross(right_, forward_));
}

void Camera::updateOrbit(GLFWwindow* window, float dt) {
    double mx, my;
    glfwGetCursorPos(window, &mx, &my);

    const bool dragging = glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS;
    if (dragging) {
        if (!firstMouse_) {
            const float dx = static_cast<float>(mx - lastMouseX_);
            const float dy = static_cast<float>(my - lastMouseY_);
            orbitYaw_   += dx * mouseSensitivity_;
            orbitPitch_ -= dy * mouseSensitivity_;
            orbitPitch_ = std::clamp(orbitPitch_, -89.0f, 89.0f);
        }
        firstMouse_ = false;
    } else {
        firstMouse_ = true;
    }
    lastMouseX_ = mx;
    lastMouseY_ = my;

    if (pendingScrollDelta_ != 0.0f) {
        const float factor = std::pow(1.0f - 0.1f, pendingScrollDelta_ * orbitZoomSpeed_);
        setOrbitRadius(orbitRadius_ * factor);
    }

    rebuildBasisFromYawPitch(orbitYaw_, orbitPitch_);

    position_ = target_ - forward_ * orbitRadius_;
    forward_  = glm::normalize(target_ - position_);
    right_    = glm::normalize(glm::cross(forward_, glm::vec3(0.0f, 1.0f, 0.0f)));
    up_       = glm::normalize(glm::cross(right_, forward_));

    (void)dt;
}

void Camera::updateFreeFly(GLFWwindow* window, float dt) {
    double mx, my;
    glfwGetCursorPos(window, &mx, &my);

    const bool looking = glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_RIGHT) == GLFW_PRESS;
    if (looking) {
        if (!firstMouse_) {
            const float dx = static_cast<float>(mx - lastMouseX_);
            const float dy = static_cast<float>(my - lastMouseY_);
            freeFlyYaw_   += dx * mouseSensitivity_;
            freeFlyPitch_ -= dy * mouseSensitivity_;
            freeFlyPitch_ = std::clamp(freeFlyPitch_, -89.0f, 89.0f);
        }
        firstMouse_ = false;
    } else {
        firstMouse_ = true;
    }
    lastMouseX_ = mx;
    lastMouseY_ = my;

    rebuildBasisFromYawPitch(freeFlyYaw_, freeFlyPitch_);

    float speed = moveSpeed_ * dt;
    if (glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS) speed *= 4.0f;

    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS) freeFlyPos_ += forward_ * speed;
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS) freeFlyPos_ -= forward_ * speed;
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS) freeFlyPos_ += right_   * speed;
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS) freeFlyPos_ -= right_   * speed;
    if (glfwGetKey(window, GLFW_KEY_E) == GLFW_PRESS) freeFlyPos_ += up_      * speed;
    if (glfwGetKey(window, GLFW_KEY_Q) == GLFW_PRESS) freeFlyPos_ -= up_      * speed;

    position_ = freeFlyPos_;
}

CameraGPU Camera::toGPU() const noexcept {
    CameraGPU gpu;
    gpu.position     = position_;
    gpu.forward      = forward_;
    gpu.right        = right_;
    gpu.up           = up_;
    gpu.tanHalfFovY  = std::tan(fovY_ * 0.5f);
    gpu.aspect       = aspect_;
    return gpu;
}

} // namespace bh::scene
