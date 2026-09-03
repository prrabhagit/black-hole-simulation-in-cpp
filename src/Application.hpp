#pragma once

#include "Window.hpp"
#include "Camera.hpp"
#include "ShaderProgram.hpp"
#include "FullscreenTriangle.hpp"
#include "UniformBuffer.hpp"
#include "Texture2D.hpp"
#include "Framebuffer.hpp"
#include "GpuTimer.hpp"
#include "BlackHoleParams.hpp"
#include "DiskParams.hpp"

#include <memory>

namespace bh::app {

class Application {
public:
    Application();
    ~Application();

    void run();

private:
    void initImGui();
    void shutdownImGui();
    void drawUI();
    void runValidationSuite();
    void measureShadowRadius();

    std::unique_ptr<core::Window>            window_;
    std::unique_ptr<scene::Camera>           camera_;

    std::unique_ptr<physics::BlackHoleParams> blackHole_;
    std::unique_ptr<gfx::UniformBuffer>       blackHoleUBO_;

    std::unique_ptr<physics::DiskParams>      disk_;
    std::unique_ptr<gfx::UniformBuffer>       diskUBO_;

    std::unique_ptr<gfx::UniformBuffer>       cameraUBO_;
    std::unique_ptr<gfx::Texture2D>           environmentMap_;

    std::unique_ptr<gfx::ShaderProgram>       rayTraceShader_;
    std::unique_ptr<gfx::ShaderProgram>       brightPassShader_;
    std::unique_ptr<gfx::ShaderProgram>       blurShader_;
    std::unique_ptr<gfx::ShaderProgram>       compositeShader_;
    std::unique_ptr<gfx::FullscreenTriangle>  fullscreenTri_;

    std::unique_ptr<gfx::Framebuffer> hdrBuffer_;
    std::unique_ptr<gfx::Framebuffer> brightBuffer_;
    std::unique_ptr<gfx::Framebuffer> blurBufferA_;
    std::unique_ptr<gfx::Framebuffer> blurBufferB_;

    std::unique_ptr<gfx::GpuTimer> gpuTimerRayTrace_;

    // Simulation / rendering parameters (all exposed via the ImGui panel)
    int   integrationMaxSteps_ = 128;
    float integrationBaseStep_ = 0.02f;
    float weakFieldThresholdMultiplier_ = 30.0f;
    float weakFieldBThreshold_ = 1.0f;
    bool  progradeDisk_ = true;
    int   debugMode_ = 0;
    float exposure_ = 1.0f;
    float bloomThreshold_ = 1.0f;
    float bloomStrength_ = 0.6f;

    struct ValidationReport {
        bool hasResult = false;
        double measuredShadowAngle = 0.0;
        double predictedShadowAngle = 0.0;
        double weakFieldDeflectionRef = 0.0;
    } validationReport_;
};

} // namespace bh::app
