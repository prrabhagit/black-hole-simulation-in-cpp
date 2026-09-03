#include "Window.hpp"
#include "GLDebug.hpp"

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <stdexcept>
#include <cstdio>
#include <utility>

namespace {

void glfwErrorCallback(int error, const char* description) {
    std::fprintf(stderr, "[GLFW error %d] %s\n", error, description);
}

void framebufferSizeCallback(GLFWwindow* window, int width, int height) {
    auto* self = static_cast<bh::core::Window*>(glfwGetWindowUserPointer(window));
    if (self) self->onFramebufferResize(width, height);
}

void scrollCallback(GLFWwindow* window, double xoffset, double yoffset) {
    auto* self = static_cast<bh::core::Window*>(glfwGetWindowUserPointer(window));
    if (self) self->onScroll(xoffset, yoffset);
}

} // namespace

namespace bh::core {

Window::Window(const WindowDesc& desc)
    : width_(desc.width), height_(desc.height), vsync_(desc.vsync) {

    glfwSetErrorCallback(glfwErrorCallback);

    if (!glfwInit()) {
        throw std::runtime_error("Failed to initialize GLFW");
    }

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GLFW_TRUE);
    glfwWindowHint(GLFW_RESIZABLE, desc.resizable ? GLFW_TRUE : GLFW_FALSE);
    glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, desc.debugContext ? GLFW_TRUE : GLFW_FALSE);

    window_ = glfwCreateWindow(desc.width, desc.height, desc.title.c_str(), nullptr, nullptr);
    if (!window_) {
        glfwTerminate();
        throw std::runtime_error("Failed to create GLFW window/context (requested OpenGL 4.6 Core)");
    }

    glfwSetWindowUserPointer(window_, this);
    glfwSetFramebufferSizeCallback(window_, framebufferSizeCallback);
    glfwSetScrollCallback(window_, scrollCallback);

    glfwMakeContextCurrent(window_);
    glfwSwapInterval(vsync_ ? 1 : 0);

    if (!gladLoadGLLoader(reinterpret_cast<GLADloadproc>(glfwGetProcAddress))) {
        glfwDestroyWindow(window_);
        glfwTerminate();
        throw std::runtime_error("GLAD failed to load OpenGL function pointers");
    }

    int fbWidth = 0, fbHeight = 0;
    glfwGetFramebufferSize(window_, &fbWidth, &fbHeight);
    width_  = fbWidth;
    height_ = fbHeight;
    glViewport(0, 0, width_, height_);

    if (desc.debugContext) {
        installGLDebugCallback();
    }

    std::fprintf(stderr, "[Window] OpenGL %s | GLSL %s | %s\n",
                 glGetString(GL_VERSION), glGetString(GL_SHADING_LANGUAGE_VERSION),
                 glGetString(GL_RENDERER));
}

Window::~Window() { destroy(); }

void Window::destroy() noexcept {
    if (window_) {
        glfwDestroyWindow(window_);
        window_ = nullptr;
    }
}

Window::Window(Window&& other) noexcept
    : window_(std::exchange(other.window_, nullptr)),
      width_(other.width_), height_(other.height_), vsync_(other.vsync_),
      resizeCallback_(std::move(other.resizeCallback_)),
      scrollCallback_(std::move(other.scrollCallback_)) {
    if (window_) glfwSetWindowUserPointer(window_, this);
}

Window& Window::operator=(Window&& other) noexcept {
    if (this != &other) {
        destroy();
        window_ = std::exchange(other.window_, nullptr);
        width_  = other.width_;
        height_ = other.height_;
        vsync_  = other.vsync_;
        resizeCallback_ = std::move(other.resizeCallback_);
        scrollCallback_ = std::move(other.scrollCallback_);
        if (window_) glfwSetWindowUserPointer(window_, this);
    }
    return *this;
}

bool Window::shouldClose() const { return glfwWindowShouldClose(window_) != 0; }
void Window::pollEvents() const { glfwPollEvents(); }
void Window::swapBuffers() const { glfwSwapBuffers(window_); }

void Window::onFramebufferResize(int newWidth, int newHeight) {
    width_ = newWidth;
    height_ = newHeight;
    glViewport(0, 0, width_, height_);
    if (resizeCallback_) resizeCallback_(newWidth, newHeight);
}

void Window::onScroll(double /*xoffset*/, double yoffset) {
    if (scrollCallback_) scrollCallback_(0.0, yoffset);
}

} // namespace bh::core
