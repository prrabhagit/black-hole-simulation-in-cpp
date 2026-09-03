#pragma once

namespace bh::gfx {

class Framebuffer {
public:
    Framebuffer(int width, int height, bool useHalfFloat = true);
    ~Framebuffer();

    Framebuffer(const Framebuffer&)            = delete;
    Framebuffer& operator=(const Framebuffer&) = delete;
    Framebuffer(Framebuffer&& other) noexcept;
    Framebuffer& operator=(Framebuffer&& other) noexcept;

    void bind() const noexcept;
    static void bindDefault() noexcept;

    void resize(int width, int height);

    [[nodiscard]] unsigned int colorTexture() const noexcept { return colorTex_; }
    [[nodiscard]] int width()  const noexcept { return width_; }
    [[nodiscard]] int height() const noexcept { return height_; }

private:
    void create();
    void destroy() noexcept;

    unsigned int fbo_ = 0;
    unsigned int colorTex_ = 0;
    int width_ = 0, height_ = 0;
    bool useHalfFloat_ = true;
};

} // namespace bh::gfx
