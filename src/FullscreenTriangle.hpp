#pragma once

namespace bh::gfx {

class FullscreenTriangle {
public:
    FullscreenTriangle();
    ~FullscreenTriangle();

    FullscreenTriangle(const FullscreenTriangle&)            = delete;
    FullscreenTriangle& operator=(const FullscreenTriangle&) = delete;
    FullscreenTriangle(FullscreenTriangle&& other) noexcept;
    FullscreenTriangle& operator=(FullscreenTriangle&& other) noexcept;

    void draw() const noexcept;

private:
    void destroy() noexcept;
    unsigned int vao_ = 0;
};

} // namespace bh::gfx
