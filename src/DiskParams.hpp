#pragma once
#include <glm/glm.hpp>

namespace bh::physics {

struct DiskParamsGPU {
    glm::vec3 normal;        float innerRadius;
    float outerRadius;       float peakTempKelvin;
    float _pad0, _pad1;
};

class DiskParams {
public:
    explicit DiskParams(float innerRadius, float outerRadius = 0.0f,
                         glm::vec3 normal = glm::vec3(0.0f, 1.0f, 0.0f))
        : normal_(glm::normalize(normal)), innerRadius_(innerRadius),
          outerRadius_(outerRadius > 0.0f ? outerRadius : innerRadius * 6.0f),
          peakTempKelvin_(20000.0f) {}

    void setPeakTemperature(float k) noexcept { peakTempKelvin_ = k; }
    [[nodiscard]] float peakTemperature() const noexcept { return peakTempKelvin_; }
    [[nodiscard]] float innerRadius() const noexcept { return innerRadius_; }
    [[nodiscard]] float outerRadius() const noexcept { return outerRadius_; }

    [[nodiscard]] DiskParamsGPU toGPU() const noexcept {
        DiskParamsGPU gpu{};
        gpu.normal = normal_;
        gpu.innerRadius = innerRadius_;
        gpu.outerRadius = outerRadius_;
        gpu.peakTempKelvin = peakTempKelvin_;
        return gpu;
    }

private:
    glm::vec3 normal_;
    float innerRadius_, outerRadius_, peakTempKelvin_;
};

} // namespace bh::physics
