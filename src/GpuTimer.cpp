#include "GpuTimer.hpp"
#include <glad/glad.h>

namespace bh::gfx {

GpuTimer::GpuTimer() { glGenQueries(2, queries_); }
GpuTimer::~GpuTimer() { glDeleteQueries(2, queries_); }

void GpuTimer::begin() { glBeginQuery(GL_TIME_ELAPSED, queries_[writeIndex_]); }
void GpuTimer::end() {
    glEndQuery(GL_TIME_ELAPSED);
    const int readIndex = 1 - writeIndex_;
    GLint available = 0;
    glGetQueryObjectiv(queries_[readIndex], GL_QUERY_RESULT_AVAILABLE, &available);
    if (available) {
        GLuint64 ns = 0;
        glGetQueryObjectui64v(queries_[readIndex], GL_QUERY_RESULT, &ns);
        lastResultMs_ = static_cast<double>(ns) / 1e6;
    }
    writeIndex_ = readIndex;
}

} // namespace bh::gfx
