#include "UniformBuffer.hpp"
#include <glad/glad.h>
#include <utility>

namespace bh::gfx {

UniformBuffer::UniformBuffer(unsigned int bindingPoint, std::size_t sizeBytes)
    : bindingPoint_(bindingPoint) {
    glGenBuffers(1, &ubo_);
    glBindBuffer(GL_UNIFORM_BUFFER, ubo_);
    glBufferData(GL_UNIFORM_BUFFER, static_cast<GLsizeiptr>(sizeBytes), nullptr, GL_DYNAMIC_DRAW);
    glBindBufferBase(GL_UNIFORM_BUFFER, bindingPoint_, ubo_);
    glBindBuffer(GL_UNIFORM_BUFFER, 0);
}

UniformBuffer::~UniformBuffer() { destroy(); }

void UniformBuffer::destroy() noexcept {
    if (ubo_ != 0) {
        glDeleteBuffers(1, &ubo_);
        ubo_ = 0;
    }
}

UniformBuffer::UniformBuffer(UniformBuffer&& other) noexcept
    : ubo_(std::exchange(other.ubo_, 0)), bindingPoint_(other.bindingPoint_) {}

UniformBuffer& UniformBuffer::operator=(UniformBuffer&& other) noexcept {
    if (this != &other) {
        destroy();
        ubo_ = std::exchange(other.ubo_, 0);
        bindingPoint_ = other.bindingPoint_;
    }
    return *this;
}

void UniformBuffer::update(const void* data, std::size_t sizeBytes) const {
    glBindBuffer(GL_UNIFORM_BUFFER, ubo_);
    glBufferSubData(GL_UNIFORM_BUFFER, 0, static_cast<GLsizeiptr>(sizeBytes), data);
    glBindBuffer(GL_UNIFORM_BUFFER, 0);
}

} // namespace bh::gfx
