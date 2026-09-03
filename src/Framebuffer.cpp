#include "Framebuffer.hpp"
#include <glad/glad.h>
#include <stdexcept>
#include <utility>

namespace bh::gfx {

Framebuffer::Framebuffer(int width, int height, bool useHalfFloat)
    : width_(width), height_(height), useHalfFloat_(useHalfFloat) {
    create();
}

void Framebuffer::create() {
    glGenFramebuffers(1, &fbo_);
    glBindFramebuffer(GL_FRAMEBUFFER, fbo_);

    glGenTextures(1, &colorTex_);
    glBindTexture(GL_TEXTURE_2D, colorTex_);
    const GLenum internalFormat = useHalfFloat_ ? GL_RGBA16F : GL_RGBA32F;
    glTexImage2D(GL_TEXTURE_2D, 0, internalFormat, width_, height_, 0, GL_RGBA, GL_FLOAT, nullptr);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, colorTex_, 0);

    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        destroy();
        throw std::runtime_error("Framebuffer incomplete");
    }
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

Framebuffer::~Framebuffer() { destroy(); }

void Framebuffer::destroy() noexcept {
    if (colorTex_ != 0) { glDeleteTextures(1, &colorTex_); colorTex_ = 0; }
    if (fbo_ != 0)      { glDeleteFramebuffers(1, &fbo_);   fbo_ = 0; }
}

Framebuffer::Framebuffer(Framebuffer&& other) noexcept
    : fbo_(std::exchange(other.fbo_, 0)), colorTex_(std::exchange(other.colorTex_, 0)),
      width_(other.width_), height_(other.height_), useHalfFloat_(other.useHalfFloat_) {}

Framebuffer& Framebuffer::operator=(Framebuffer&& other) noexcept {
    if (this != &other) {
        destroy();
        fbo_ = std::exchange(other.fbo_, 0);
        colorTex_ = std::exchange(other.colorTex_, 0);
        width_ = other.width_; height_ = other.height_; useHalfFloat_ = other.useHalfFloat_;
    }
    return *this;
}

void Framebuffer::bind() const noexcept {
    glBindFramebuffer(GL_FRAMEBUFFER, fbo_);
    glViewport(0, 0, width_, height_);
}

void Framebuffer::bindDefault() noexcept {
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void Framebuffer::resize(int width, int height) {
    if (width == width_ && height == height_) return;
    destroy();
    width_ = width; height_ = height;
    create();
}

} // namespace bh::gfx
