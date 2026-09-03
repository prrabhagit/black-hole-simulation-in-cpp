#pragma once

namespace bh::core {

/// Installs a GL_KHR_debug callback routing GL errors/warnings to stderr.
/// Must be called after a GL context is current and GLAD has loaded.
void installGLDebugCallback();

} // namespace bh::core
