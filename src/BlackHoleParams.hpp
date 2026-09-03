#pragma once
#include <glm/glm.hpp>
#include <algorithm>

namespace bh::physics {

struct BlackHoleParamsGPU {
    glm::vec3 position;          float mass;
    float schwarzschildRadius;   float photonSphereRadius;
    float _pad0, _pad1;
};

class BlackHoleParams {
public:
    explicit BlackHoleParams(float mass = 1.0f, glm::vec3 position = glm::vec3(0.0f))
        : position_(position), mass_(std::max(mass, 0.0f)) {}

    void setMass(float mass) noexcept { mass_ = std::max(mass, 0.01f); }
    [[nodiscard]] float mass() const noexcept { return mass_; }

    void setPosition(const glm::vec3& p) noexcept { position_ = p; }
    [[nodiscard]] glm::vec3 position() const noexcept { return position_; }

    [[nodiscard]] float schwarzschildRadius() const noexcept { return 2.0f * mass_; }
    [[nodiscard]] float photonSphereRadius() const noexcept { return 1.5f * schwarzschildRadius(); }

    [[nodiscard]] BlackHoleParamsGPU toGPU() const noexcept {
        BlackHoleParamsGPU gpu{};
        gpu.position = position_;
        gpu.mass = mass_;
        gpu.schwarzschildRadius = schwarzschildRadius();
        gpu.photonSphereRadius = photonSphereRadius();
        return gpu;
    }

private:
    glm::vec3 position_;
    float mass_;
};

} // namespace bh::physics
