#include "Application.hpp"
#include "BindingPoints.hpp"
#include "SchwarzschildMetric.hpp"

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>

#include <algorithm>
#include <cmath>
#include <cstdio>
#include <vector>

namespace bh::app {

Application::Application() {
    core::WindowDesc desc;
    desc.width = 1920; desc.height = 1080;
    desc.title = "Schwarzschild Black Hole Renderer";
    desc.vsync = true; desc.debugContext = true;

    window_ = std::make_unique<core::Window>(desc);
    camera_ = std::make_unique<scene::Camera>(window_->aspect());

    window_->setResizeCallback([this](int w, int h) {
        if (h <= 0) return;
        camera_->setAspect(static_cast<float>(w) / static_cast<float>(h));
        if (hdrBuffer_)    hdrBuffer_->resize(w, h);
        if (brightBuffer_) brightBuffer_->resize(w / 2, h / 2);
        if (blurBufferA_)  blurBufferA_->resize(w / 2, h / 2);
        if (blurBufferB_)  blurBufferB_->resize(w / 2, h / 2);
    });
    window_->setScrollCallback([this](double /*x*/, double y) {
        camera_->feedScrollDelta(static_cast<float>(y));
    });

    blackHole_ = std::make_unique<physics::BlackHoleParams>(/*mass=*/1.0f);
    blackHoleUBO_ = std::make_unique<gfx::UniformBuffer>(
        gfx::kBlackHoleUBOBinding, sizeof(physics::BlackHoleParamsGPU));

    disk_ = std::make_unique<physics::DiskParams>(
        blackHole_->schwarzschildRadius() * 3.0f,
        blackHole_->schwarzschildRadius() * 3.0f * 6.0f);
    diskUBO_ = std::make_unique<gfx::UniformBuffer>(
        gfx::kDiskUBOBinding, sizeof(physics::DiskParamsGPU));

    camera_->setOrbitRadius(std::max(camera_->orbitRadius(),
                                      blackHole_->schwarzschildRadius() * 3.0f));
    weakFieldBThreshold_ = weakFieldThresholdMultiplier_ * blackHole_->schwarzschildRadius();

    cameraUBO_ = std::make_unique<gfx::UniformBuffer>(
        gfx::kCameraUBOBinding, sizeof(scene::CameraGPU));

    // NOTE: place a real equirectangular HDR starfield at textures/starfield.hdr
    // before running -- see README.md.
    environmentMap_ = std::make_unique<gfx::Texture2D>("textures/starfield.hdr");

    rayTraceShader_   = std::make_unique<gfx::ShaderProgram>("shaders/fullscreen.vert", "shaders/raytrace.frag");
    brightPassShader_ = std::make_unique<gfx::ShaderProgram>("shaders/fullscreen.vert", "shaders/brightpass.frag");
    blurShader_       = std::make_unique<gfx::ShaderProgram>("shaders/fullscreen.vert", "shaders/blur.frag");
    compositeShader_  = std::make_unique<gfx::ShaderProgram>("shaders/fullscreen.vert", "shaders/composite.frag");
    fullscreenTri_    = std::make_unique<gfx::FullscreenTriangle>();

    const int bw = window_->width(), bh = window_->height();
    hdrBuffer_    = std::make_unique<gfx::Framebuffer>(bw, bh);
    brightBuffer_ = std::make_unique<gfx::Framebuffer>(std::max(bw / 2, 1), std::max(bh / 2, 1));
    blurBufferA_  = std::make_unique<gfx::Framebuffer>(std::max(bw / 2, 1), std::max(bh / 2, 1));
    blurBufferB_  = std::make_unique<gfx::Framebuffer>(std::max(bw / 2, 1), std::max(bh / 2, 1));

    gpuTimerRayTrace_ = std::make_unique<gfx::GpuTimer>();

    initImGui();
}

Application::~Application() {
    shutdownImGui();
    hdrBuffer_.reset(); brightBuffer_.reset(); blurBufferA_.reset(); blurBufferB_.reset();
    rayTraceShader_.reset(); brightPassShader_.reset(); blurShader_.reset(); compositeShader_.reset();
    fullscreenTri_.reset();
    environmentMap_.reset();
    cameraUBO_.reset(); blackHoleUBO_.reset(); diskUBO_.reset();
    blackHole_.reset(); disk_.reset(); camera_.reset();
    gpuTimerRayTrace_.reset();
    window_.reset();
    glfwTerminate();
}

void Application::initImGui() {
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGui::StyleColorsDark();
    ImGui_ImplGlfw_InitForOpenGL(window_->handle(), true);
    ImGui_ImplOpenGL3_Init("#version 460");
}

void Application::shutdownImGui() {
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();
}

void Application::measureShadowRadius() {
    const int w = window_->width();
    const int h = window_->height();
    const int centerRow = h / 2;
    if (w <= 0 || h <= 0) return;

    std::vector<unsigned char> rowPixels(static_cast<size_t>(w) * 3);
    glReadPixels(0, centerRow, w, 1, GL_RGB, GL_UNSIGNED_BYTE, rowPixels.data());

    const int centerX = w / 2;
    const unsigned char kBlackThreshold = 5;
    int edgeOffsetPx = 0;
    for (int dx = 0; dx < centerX; ++dx) {
        const int px = centerX + dx;
        const size_t idx = static_cast<size_t>(px) * 3;
        const bool isBlack = rowPixels[idx] < kBlackThreshold &&
                              rowPixels[idx + 1] < kBlackThreshold &&
                              rowPixels[idx + 2] < kBlackThreshold;
        if (!isBlack) { edgeOffsetPx = dx; break; }
    }

    const float fovY = camera_->fovYRadians();
    const float tanHalfFovY = std::tan(fovY * 0.5f);
    const scene::CameraGPU camGpu = camera_->toGPU();
    const float ndcX = (2.0f * static_cast<float>(edgeOffsetPx) / static_cast<float>(w))
                        * camGpu.aspect * tanHalfFovY;
    const double measuredAngle = std::atan(static_cast<double>(ndcX));

    validationReport_.measuredShadowAngle = measuredAngle;
    validationReport_.predictedShadowAngle = physics::schwarzschild::exactShadowAngularRadius(
        camera_->orbitRadius(), blackHole_->mass());
}

void Application::runValidationSuite() {
    measureShadowRadius();
    validationReport_.weakFieldDeflectionRef = physics::schwarzschild::weakFieldDeflection(
        50.0 * blackHole_->schwarzschildRadius(), blackHole_->mass());
    validationReport_.hasResult = true;
}

void Application::drawUI() {
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();

    ImGui::Begin("Black Hole Simulation");

    if (ImGui::CollapsingHeader("Black Hole", ImGuiTreeNodeFlags_DefaultOpen)) {
        float mass = blackHole_->mass();
        if (ImGui::SliderFloat("Mass (M)", &mass, 0.1f, 20.0f, "%.2f")) {
            blackHole_->setMass(mass);
            disk_ = std::make_unique<physics::DiskParams>(
                blackHole_->schwarzschildRadius() * 3.0f,
                blackHole_->schwarzschildRadius() * 3.0f * 6.0f,
                glm::vec3(0.0f, 1.0f, 0.0f));
            disk_->setPeakTemperature(disk_->peakTemperature());
            weakFieldBThreshold_ = weakFieldThresholdMultiplier_ * blackHole_->schwarzschildRadius();
            camera_->setOrbitRadius(std::max(camera_->orbitRadius(),
                                              blackHole_->schwarzschildRadius() * 3.0f));
        }
        ImGui::Text("r_s = %.3f | r_photon = %.3f | b_crit = %.3f",
                    blackHole_->schwarzschildRadius(), blackHole_->photonSphereRadius(),
                    3.0f * std::sqrt(3.0f) * blackHole_->mass());
    }

    if (ImGui::CollapsingHeader("Integration Quality", ImGuiTreeNodeFlags_DefaultOpen)) {
        ImGui::SliderInt("Max RK4 steps", &integrationMaxSteps_, 8, 512);
        ImGui::SliderFloat("Base step (rad)", &integrationBaseStep_, 0.002f, 0.08f, "%.4f");
        if (ImGui::SliderFloat("Weak-field threshold (x r_s)", &weakFieldThresholdMultiplier_, 5.0f, 60.0f)) {
            weakFieldBThreshold_ = weakFieldThresholdMultiplier_ * blackHole_->schwarzschildRadius();
        }
    }

    if (ImGui::CollapsingHeader("Accretion Disk")) {
        float peakTemp = disk_->peakTemperature();
        if (ImGui::SliderFloat("Peak Temperature (K)", &peakTemp, 2000.0f, 40000.0f)) {
            disk_->setPeakTemperature(peakTemp);
        }
        ImGui::Checkbox("Prograde rotation", &progradeDisk_);
    }

    if (ImGui::CollapsingHeader("Camera")) {
        const char* modeNames[] = { "Orbit", "Free-fly" };
        int modeIdx = camera_->mode() == scene::CameraMode::Orbit ? 0 : 1;
        if (ImGui::Combo("Mode", &modeIdx, modeNames, 2)) {
            camera_->setMode(modeIdx == 0 ? scene::CameraMode::Orbit : scene::CameraMode::FreeFly);
        }
        float orbitR = camera_->orbitRadius();
        if (ImGui::SliderFloat("Orbit distance", &orbitR, blackHole_->schwarzschildRadius() * 3.0f, 500.0f)) {
            camera_->setOrbitRadius(orbitR);
        }
    }

    if (ImGui::CollapsingHeader("Rendering")) {
        const char* debugNames[] = { "Normal", "Photon sphere overlay", "Image order overlay" };
        ImGui::Combo("Debug mode", &debugMode_, debugNames, 3);
        ImGui::SliderFloat("Exposure", &exposure_, 0.05f, 5.0f, "%.2f");
        ImGui::SliderFloat("Bloom threshold", &bloomThreshold_, 0.1f, 5.0f);
        ImGui::SliderFloat("Bloom strength", &bloomStrength_, 0.0f, 2.0f);
    }

    if (ImGui::CollapsingHeader("Validation", ImGuiTreeNodeFlags_DefaultOpen)) {
        if (ImGui::Button("Run Validation Suite")) {
            runValidationSuite();
        }
        if (validationReport_.hasResult) {
            const double diffPct = 100.0 * std::abs(
                validationReport_.measuredShadowAngle - validationReport_.predictedShadowAngle) /
                validationReport_.predictedShadowAngle;
            ImGui::Text("Shadow angle: measured=%.6f rad, predicted=%.6f rad (%.3f%% diff)",
                        validationReport_.measuredShadowAngle,
                        validationReport_.predictedShadowAngle, diffPct);
            ImGui::Text("Weak-field deflection ref (b=50 r_s): %.6f rad",
                        validationReport_.weakFieldDeflectionRef);
            if (diffPct < 1.0) {
                ImGui::TextColored(ImVec4(0.3f, 1.0f, 0.4f, 1.0f), "PASS: within 1%% of exact prediction");
            } else {
                ImGui::TextColored(ImVec4(1.0f, 0.6f, 0.2f, 1.0f), "Consider increasing max RK4 steps");
            }
        }
    }

    ImGui::Text("Frame: %.2f ms (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);
    ImGui::Text("Ray trace GPU time: %.3f ms", gpuTimerRayTrace_->lastResultMs());

    ImGui::End();
    ImGui::Render();
}

void Application::run() {
    double lastTime = glfwGetTime();
    while (!window_->shouldClose()) {
        const double now = glfwGetTime();
        const float dt = static_cast<float>(now - lastTime);
        lastTime = now;

        window_->pollEvents();

        ImGuiIO& io = ImGui::GetIO();
        if (!io.WantCaptureMouse && !io.WantCaptureKeyboard) {
            camera_->update(window_->handle(), dt);
        }

        const scene::CameraGPU camGPU = camera_->toGPU();
        cameraUBO_->update(&camGPU, sizeof(camGPU));

        const physics::BlackHoleParamsGPU bhGPU = blackHole_->toGPU();
        blackHoleUBO_->update(&bhGPU, sizeof(bhGPU));

        const physics::DiskParamsGPU diskGPU = disk_->toGPU();
        diskUBO_->update(&diskGPU, sizeof(diskGPU));

        // 1. Ray trace pass -> HDR buffer
        gpuTimerRayTrace_->begin();
        hdrBuffer_->bind();
        glClear(GL_COLOR_BUFFER_BIT);
        rayTraceShader_->use();
        rayTraceShader_->setVec2("uResolution", glm::vec2(hdrBuffer_->width(), hdrBuffer_->height()));
        rayTraceShader_->setInt("uMaxSteps", integrationMaxSteps_);
        rayTraceShader_->setFloat("uBaseStepPhi", integrationBaseStep_);
        rayTraceShader_->setFloat("uWeakFieldBThreshold", weakFieldBThreshold_);
        rayTraceShader_->setInt("uDebugMode", debugMode_);
        rayTraceShader_->setInt("uProgradeDisk", progradeDisk_ ? 1 : 0);
        environmentMap_->bind(0);
        rayTraceShader_->setInt("uEnvironmentMap", 0);
        fullscreenTri_->draw();
        gpuTimerRayTrace_->end();

        // 2. Bright-pass extraction (downsampled)
        brightBuffer_->bind();
        glClear(GL_COLOR_BUFFER_BIT);
        brightPassShader_->use();
        brightPassShader_->setVec2("uResolution", glm::vec2(brightBuffer_->width(), brightBuffer_->height()));
        brightPassShader_->setFloat("uBloomThreshold", bloomThreshold_);
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, hdrBuffer_->colorTexture());
        brightPassShader_->setInt("uHDRColor", 0);
        fullscreenTri_->draw();

        // 3. Ping-pong Gaussian blur
        bool horizontal = true;
        gfx::Framebuffer* blurSrc = brightBuffer_.get();
        constexpr int kBlurIterations = 6;
        for (int i = 0; i < kBlurIterations; ++i) {
            gfx::Framebuffer& dst = horizontal ? *blurBufferA_ : *blurBufferB_;
            dst.bind();
            glClear(GL_COLOR_BUFFER_BIT);
            blurShader_->use();
            blurShader_->setVec2("uResolution", glm::vec2(dst.width(), dst.height()));
            blurShader_->setVec2("uDirection", horizontal ? glm::vec2(1, 0) : glm::vec2(0, 1));
            glActiveTexture(GL_TEXTURE0);
            glBindTexture(GL_TEXTURE_2D, blurSrc->colorTexture());
            blurShader_->setInt("uSource", 0);
            fullscreenTri_->draw();
            blurSrc = &dst;
            horizontal = !horizontal;
        }

        // 4. Composite: HDR + bloom -> tonemap -> gamma -> screen
        gfx::Framebuffer::bindDefault();
        glViewport(0, 0, window_->width(), window_->height());
        glClear(GL_COLOR_BUFFER_BIT);
        compositeShader_->use();
        compositeShader_->setVec2("uResolution", glm::vec2(window_->width(), window_->height()));
        compositeShader_->setFloat("uExposure", exposure_);
        compositeShader_->setFloat("uBloomStrength", bloomStrength_);
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, hdrBuffer_->colorTexture());
        compositeShader_->setInt("uHDRColor", 0);
        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_2D, blurSrc->colorTexture());
        compositeShader_->setInt("uBloom", 1);
        fullscreenTri_->draw();

        drawUI();
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

        window_->swapBuffers();
    }
}

} // namespace bh::app
