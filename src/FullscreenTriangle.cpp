#include "FullscreenTriangle.hpp"
#include <glad/glad.h>
#include <utility>

namespace bh::gfx {

FullscreenTriangle::FullscreenTriangle() {
    glGenVertexArrays(1, &vao_);
}

FullscreenTriangle::~FullscreenTriangle() { destroy(); }

void FullscreenTriangle::destroy() noexcept {
    if (vao_ != 0) {
        glDeleteVertexArrays(1, &vao_);
        vao_ = 0;
    }
}

FullscreenTriangle::FullscreenTriangle(FullscreenTriangle&& other) noexcept
    : vao_(std::exchange(other.vao_, 0)) {}

FullscreenTriangle& FullscreenTriangle::operator=(FullscreenTriangle&& other) noexcept {
    if (this != &other) {
        destroy();
        vao_ = std::exchange(other.vao_, 0);
    }
    return *this;
}

void FullscreenTriangle::draw() const noexcept {
    glBindVertexArray(vao_);
    glDrawArrays(GL_TRIANGLES, 0, 3);
}

} // namespace bh::gfx
