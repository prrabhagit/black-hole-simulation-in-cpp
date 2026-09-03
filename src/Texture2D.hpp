#pragma once
#include <string>

namespace bh::gfx {

class Texture2D {
public:
    explicit Texture2D(const std::string& hdrPath, bool use16Bit = true);
    ~Texture2D();

    Texture2D(const Texture2D&)            = delete;
    Texture2D& operator=(const Texture2D&) = delete;
    Texture2D(Texture2D&& other) noexcept;
    Texture2D& operator=(Texture2D&& other) noexcept;

    void bind(unsigned int unit) const noexcept;

    [[nodiscard]] int width()  const noexcept { return width_; }
    [[nodiscard]] int height() const noexcept { return height_; }

private:
    void destroy() noexcept;
    unsigned int tex_ = 0;
    int width_ = 0, height_ = 0;
};

} // namespace bh::gfx
