#pragma once
#include <cstddef>

namespace bh::gfx {

class UniformBuffer {
public:
    UniformBuffer(unsigned int bindingPoint, std::size_t sizeBytes);
    ~UniformBuffer();

    UniformBuffer(const UniformBuffer&)            = delete;
    UniformBuffer& operator=(const UniformBuffer&) = delete;
    UniformBuffer(UniformBuffer&& other) noexcept;
    UniformBuffer& operator=(UniformBuffer&& other) noexcept;

    void update(const void* data, std::size_t sizeBytes) const;

private:
    void destroy() noexcept;
    unsigned int ubo_ = 0;
    unsigned int bindingPoint_ = 0;
};

} // namespace bh::gfx
