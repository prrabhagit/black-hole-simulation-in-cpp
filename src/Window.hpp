#pragma once

#include <string>
#include <functional>

struct GLFWwindow;

namespace bh::core {

struct WindowDesc {
    int         width      = 1920;
    int         height     = 1080;
    std::string title      = "Black Hole Simulation";
    bool        vsync      = true;
    bool        resizable  = true;
    bool        debugContext = true;
};

class Window {
public:
    explicit Window(const WindowDesc& desc);
    ~Window();

    Window(const Window&)            = delete;
    Window& operator=(const Window&) = delete;
    Window(Window&& other) noexcept;
    Window& operator=(Window&& other) noexcept;

    [[nodiscard]] bool shouldClose() const;
    void pollEvents() const;
    void swapBuffers() const;

    [[nodiscard]] int   width()  const noexcept { return width_; }
    [[nodiscard]] int   height() const noexcept { return height_; }
    [[nodiscard]] float aspect() const noexcept {
        return height_ > 0 ? static_cast<float>(width_) / static_cast<float>(height_) : 1.0f;
    }

    [[nodiscard]] GLFWwindow* handle() const noexcept { return window_; }

    void onFramebufferResize(int newWidth, int newHeight);
    void onScroll(double xoffset, double yoffset);

    void setResizeCallback(std::function<void(int, int)> cb) { resizeCallback_ = std::move(cb); }
    void setScrollCallback(std::function<void(double, double)> cb) { scrollCallback_ = std::move(cb); }

private:
    void destroy() noexcept;

    GLFWwindow* window_ = nullptr;
    int width_  = 0;
    int height_ = 0;
    bool vsync_ = true;
    std::function<void(int, int)> resizeCallback_;
    std::function<void(double, double)> scrollCallback_;
};

} // namespace bh::core
