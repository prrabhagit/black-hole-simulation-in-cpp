#pragma once

namespace bh::gfx {

class GpuTimer {
public:
    GpuTimer();
    ~GpuTimer();
    GpuTimer(const GpuTimer&) = delete;
    GpuTimer& operator=(const GpuTimer&) = delete;

    void begin();
    void end();

    [[nodiscard]] double lastResultMs() const noexcept { return lastResultMs_; }

private:
    unsigned int queries_[2] = {0, 0};
    int writeIndex_ = 0;
    double lastResultMs_ = -1.0;
};

} // namespace bh::gfx
