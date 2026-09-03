#include "ShaderProgram.hpp"

#include <glad/glad.h>
#include <fstream>
#include <sstream>
#include <stdexcept>
#include <vector>
#include <filesystem>
#include <utility>

namespace {

std::string resolveIncludes(const std::string& path, std::vector<std::string>& includeStack) {
    for (const auto& active : includeStack) {
        if (active == path) {
            throw std::runtime_error("Circular #include detected involving: " + path);
        }
    }
    includeStack.push_back(path);

    std::ifstream file(path, std::ios::in | std::ios::binary);
    if (!file) {
        throw std::runtime_error("Failed to open shader file: " + path);
    }

    const std::filesystem::path baseDir = std::filesystem::path(path).parent_path();
    std::ostringstream out;
    std::string line;
    while (std::getline(file, line)) {
        const std::string trimmed = [&] {
            size_t start = line.find_first_not_of(" \t");
            return start == std::string::npos ? std::string() : line.substr(start);
        }();

        if (trimmed.rfind("#include", 0) == 0) {
            const size_t firstQuote = trimmed.find('"');
            const size_t lastQuote  = trimmed.rfind('"');
            if (firstQuote == std::string::npos || lastQuote == firstQuote) {
                throw std::runtime_error("Malformed #include in " + path + ": " + line);
            }
            const std::string includeRel = trimmed.substr(firstQuote + 1, lastQuote - firstQuote - 1);
            const std::string includePath = (baseDir / includeRel).string();
            out << resolveIncludes(includePath, includeStack) << '\n';
        } else {
            out << line << '\n';
        }
    }

    includeStack.pop_back();
    return out.str();
}

std::string readFile(const std::string& path) {
    std::vector<std::string> includeStack;
    return resolveIncludes(path, includeStack);
}

unsigned int compileStage(GLenum stage, const std::string& source, const std::string& debugName) {
    unsigned int shader = glCreateShader(stage);
    const char* src = source.c_str();
    glShaderSource(shader, 1, &src, nullptr);
    glCompileShader(shader);

    GLint success = 0;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
    if (!success) {
        GLint logLen = 0;
        glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &logLen);
        std::vector<char> log(static_cast<size_t>(logLen));
        glGetShaderInfoLog(shader, logLen, nullptr, log.data());
        glDeleteShader(shader);
        throw std::runtime_error("Shader compile error [" + debugName + "]:\n" +
                                  std::string(log.data(), log.size()));
    }
    return shader;
}

} // namespace

namespace bh::gfx {

ShaderProgram::ShaderProgram(const std::string& vertPath, const std::string& fragPath) {
    const std::string vertSrc = readFile(vertPath);
    const std::string fragSrc = readFile(fragPath);

    const unsigned int vs = compileStage(GL_VERTEX_SHADER, vertSrc, vertPath);
    const unsigned int fs = compileStage(GL_FRAGMENT_SHADER, fragSrc, fragPath);

    program_ = glCreateProgram();
    glAttachShader(program_, vs);
    glAttachShader(program_, fs);
    glLinkProgram(program_);

    glDeleteShader(vs);
    glDeleteShader(fs);

    GLint success = 0;
    glGetProgramiv(program_, GL_LINK_STATUS, &success);
    if (!success) {
        GLint logLen = 0;
        glGetProgramiv(program_, GL_INFO_LOG_LENGTH, &logLen);
        std::vector<char> log(static_cast<size_t>(logLen));
        glGetProgramInfoLog(program_, logLen, nullptr, log.data());
        glDeleteProgram(program_);
        program_ = 0;
        throw std::runtime_error("Shader link error [" + vertPath + " + " + fragPath + "]:\n" +
                                  std::string(log.data(), log.size()));
    }
}

ShaderProgram::~ShaderProgram() { destroy(); }

void ShaderProgram::destroy() noexcept {
    if (program_ != 0) {
        glDeleteProgram(program_);
        program_ = 0;
    }
}

ShaderProgram::ShaderProgram(ShaderProgram&& other) noexcept
    : program_(std::exchange(other.program_, 0)),
      uniformCache_(std::move(other.uniformCache_)) {}

ShaderProgram& ShaderProgram::operator=(ShaderProgram&& other) noexcept {
    if (this != &other) {
        destroy();
        program_ = std::exchange(other.program_, 0);
        uniformCache_ = std::move(other.uniformCache_);
    }
    return *this;
}

void ShaderProgram::use() const noexcept { glUseProgram(program_); }

int ShaderProgram::uniformLocation(std::string_view name) const {
    const std::string key(name);
    if (auto it = uniformCache_.find(key); it != uniformCache_.end()) {
        return it->second;
    }
    const int loc = glGetUniformLocation(program_, key.c_str());
    uniformCache_.emplace(key, loc);
    return loc;
}

void ShaderProgram::setInt(std::string_view name, int value) const {
    glUniform1i(uniformLocation(name), value);
}
void ShaderProgram::setFloat(std::string_view name, float value) const {
    glUniform1f(uniformLocation(name), value);
}
void ShaderProgram::setVec2(std::string_view name, const glm::vec2& v) const {
    glUniform2fv(uniformLocation(name), 1, glm::value_ptr(v));
}
void ShaderProgram::setVec3(std::string_view name, const glm::vec3& v) const {
    glUniform3fv(uniformLocation(name), 1, glm::value_ptr(v));
}
void ShaderProgram::setMat4(std::string_view name, const glm::mat4& m) const {
    glUniformMatrix4fv(uniformLocation(name), 1, GL_FALSE, glm::value_ptr(m));
}

int ShaderProgram::uniformBlockIndex(std::string_view name) const {
    const std::string key(name);
    const GLuint idx = glGetUniformBlockIndex(program_, key.c_str());
    return idx == GL_INVALID_INDEX ? -1 : static_cast<int>(idx);
}

} // namespace bh::gfx
