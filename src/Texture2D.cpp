#include "Texture2D.hpp"

#include <glad/glad.h>
#include "stb_image.h"

#include <stdexcept>
#include <utility>

namespace bh::gfx {

Texture2D::Texture2D(const std::string& hdrPath, bool use16Bit) {
    stbi_set_flip_vertically_on_load(true);
    int channels = 0;
    float* data = stbi_loadf(hdrPath.c_str(), &width_, &height_, &channels, 3);
    if (!data) {
        throw std::runtime_error("Failed to load HDR environment map: " + hdrPath +
                                  " (" + stbi_failure_reason() + ")");
    }

    glGenTextures(1, &tex_);
    glBindTexture(GL_TEXTURE_2D, tex_);

    const GLenum internalFormat = use16Bit ? GL_RGB16F : GL_RGB32F;
    glTexImage2D(GL_TEXTURE_2D, 0, internalFormat, width_, height_, 0,
                 GL_RGB, GL_FLOAT, data);

    stbi_image_free(data);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    glBindTexture(GL_TEXTURE_2D, 0);
}

Texture2D::~Texture2D() { destroy(); }

void Texture2D::destroy() noexcept {
    if (tex_ != 0) {
        glDeleteTextures(1, &tex_);
        tex_ = 0;
    }
}

Texture2D::Texture2D(Texture2D&& other) noexcept
    : tex_(std::exchange(other.tex_, 0)), width_(other.width_), height_(other.height_) {}

Texture2D& Texture2D::operator=(Texture2D&& other) noexcept {
    if (this != &other) {
        destroy();
        tex_ = std::exchange(other.tex_, 0);
        width_ = other.width_;
        height_ = other.height_;
    }
    return *this;
}

void Texture2D::bind(unsigned int unit) const noexcept {
    glActiveTexture(GL_TEXTURE0 + unit);
    glBindTexture(GL_TEXTURE_2D, tex_);
}

} // namespace bh::gfx
