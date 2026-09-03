#include "GLDebug.hpp"

#include <glad/glad.h>
#include <cstdio>

namespace {

const char* sourceToString(GLenum source) {
    switch (source) {
        case GL_DEBUG_SOURCE_API:             return "API";
        case GL_DEBUG_SOURCE_WINDOW_SYSTEM:    return "Window System";
        case GL_DEBUG_SOURCE_SHADER_COMPILER:  return "Shader Compiler";
        case GL_DEBUG_SOURCE_THIRD_PARTY:      return "Third Party";
        case GL_DEBUG_SOURCE_APPLICATION:      return "Application";
        case GL_DEBUG_SOURCE_OTHER:            return "Other";
        default:                               return "Unknown";
    }
}

const char* typeToString(GLenum type) {
    switch (type) {
        case GL_DEBUG_TYPE_ERROR:               return "Error";
        case GL_DEBUG_TYPE_DEPRECATED_BEHAVIOR:  return "Deprecated Behavior";
        case GL_DEBUG_TYPE_UNDEFINED_BEHAVIOR:   return "Undefined Behavior";
        case GL_DEBUG_TYPE_PORTABILITY:          return "Portability";
        case GL_DEBUG_TYPE_PERFORMANCE:          return "Performance";
        case GL_DEBUG_TYPE_MARKER:               return "Marker";
        case GL_DEBUG_TYPE_OTHER:                return "Other";
        default:                                 return "Unknown";
    }
}

const char* severityToString(GLenum severity) {
    switch (severity) {
        case GL_DEBUG_SEVERITY_HIGH:         return "HIGH";
        case GL_DEBUG_SEVERITY_MEDIUM:       return "MEDIUM";
        case GL_DEBUG_SEVERITY_LOW:          return "LOW";
        case GL_DEBUG_SEVERITY_NOTIFICATION: return "NOTIFICATION";
        default:                             return "UNKNOWN";
    }
}

void APIENTRY glDebugCallback(GLenum source, GLenum type, GLuint id,
                               GLenum severity, GLsizei /*length*/,
                               const GLchar* message, const void* /*userParam*/) {
    if (severity == GL_DEBUG_SEVERITY_NOTIFICATION) return;
    std::fprintf(stderr, "[GL %s] source=%s type=%s id=%u: %s\n",
                 severityToString(severity), sourceToString(source),
                 typeToString(type), id, message);
}

} // namespace

namespace bh::core {

void installGLDebugCallback() {
    GLint flags = 0;
    glGetIntegerv(GL_CONTEXT_FLAGS, &flags);
    if (!(flags & GL_CONTEXT_FLAG_DEBUG_BIT)) {
        std::fprintf(stderr, "[GLDebug] Context was not created with debug flag; skipping.\n");
        return;
    }
    glEnable(GL_DEBUG_OUTPUT);
    glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
    glDebugMessageCallback(glDebugCallback, nullptr);
    glDebugMessageControl(GL_DONT_CARE, GL_DONT_CARE, GL_DONT_CARE, 0, nullptr, GL_TRUE);
}

} // namespace bh::core
