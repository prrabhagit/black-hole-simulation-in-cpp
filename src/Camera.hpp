#pragma once

#include <glm/glm.hpp>

struct GLFWwindow;

namespace bh::scene {

enum class CameraMode { Orbit, FreeFly };

struct CameraGPU {
    glm::vec3 position;      float _pad0 = 0.0f;
    glm::vec3 forward;       float _pad1 = 0.0f;
    glm::vec3 right;         float _pad2 = 0.0f;
    glm::vec3 up;            float tanHalfFovY = 0.0f;
    float     aspect = 1.0f; float _pad3[3] = {0.0f, 0.0f, 0.0f};
};

class Camera {
public:
    explicit Camera(float aspect);

    void update(GLFWwindow* window, float dt);

    void setMode(CameraMode mode);
    void toggleMode();
    [[nodiscard]] CameraMode mode() const noexcept { return mode_; }

    void setAspect(float aspect) noexcept { aspect_ = aspect; }

    [[nodiscard]] glm::vec3 position() const noexcept { return position_; }
    [[nodiscard]] glm::vec3 forward()  const noexcept { return forward_; }
    [[nodiscard]] glm::vec3 right()    const noexcept { return right_; }
    [[nodiscard]] glm::vec3 up()       const noexcept { return up_; }
    [[nodiscard]] float     fovYRadians() const noexcept { return fovY_; }

    [[nodiscard]] float orbitRadius() const noexcept { return orbitRadius_; }
    void setOrbitRadius(float r) noexcept;

    [[nodiscard]] CameraGPU toGPU() const noexcept;

    void feedScrollDelta(float yOffset) noexcept { pendingScrollDelta_ += yOffset; }

private:
    void updateOrbit(GLFWwindow* window, float dt);
    void updateFreeFly(GLFWwindow* window, float dt);
    void rebuildBasisFromYawPitch(float yaw, float pitch);

    CameraMode mode_ = CameraMode::Orbit;

    glm::vec3 position_ = glm::vec3(0.0f, 0.0f, 30.0f);
    glm::vec3 forward_  = glm::vec3(0.0f, 0.0f, -1.0f);
    glm::vec3 right_    = glm::vec3(1.0f, 0.0f, 0.0f);
    glm::vec3 up_       = glm::vec3(0.0f, 1.0f, 0.0f);

    float fovY_   = glm::radians(50.0f);
    float aspect_ = 16.0f / 9.0f;

    glm::vec3 target_       = glm::vec3(0.0f);
    float orbitRadius_      = 30.0f;
    float orbitYaw_         = 0.0f;
    float orbitPitch_       = 0.3f;
    float minRadius_        = 3.0f;
    float maxRadius_        = 500.0f;
    float orbitZoomSpeed_   = 2.0f;

    glm::vec3 freeFlyPos_   = glm::vec3(0.0f, 2.0f, 30.0f);
    float freeFlyYaw_       = -90.0f;
    float freeFlyPitch_     = 0.0f;
    float moveSpeed_        = 10.0f;

    double lastMouseX_ = 0.0, lastMouseY_ = 0.0;
    bool firstMouse_   = true;
    float mouseSensitivity_ = 0.12f;

    float pendingScrollDelta_ = 0.0f;
};

} // namespace bh::scene
