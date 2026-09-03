#pragma once

#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <string>
#include <string_view>
#include <unordered_map>

namespace bh::gfx {

class ShaderProgram {
public:
    ShaderProgram(const std::string& vertPath, const std::string& fragPath);
    ~ShaderProgram();

    ShaderProgram(const ShaderProgram&)            = delete;
    ShaderProgram& operator=(const ShaderProgram&) = delete;
    ShaderProgram(ShaderProgram&& other) noexcept;
    ShaderProgram& operator=(ShaderProgram&& other) noexcept;

    void use() const noexcept;

    void setInt(std::string_view name, int value) const;
    void setFloat(std::string_view name, float value) const;
    void setVec2(std::string_view name, const glm::vec2& v) const;
    void setVec3(std::string_view name, const glm::vec3& v) const;
    void setMat4(std::string_view name, const glm::mat4& m) const;

    [[nodiscard]] unsigned int id() const noexcept { return program_; }
    [[nodiscard]] int uniformBlockIndex(std::string_view name) const;

private:
    [[nodiscard]] int uniformLocation(std::string_view name) const;
    void destroy() noexcept;

    unsigned int program_ = 0;
    mutable std::unordered_map<std::string, int> uniformCache_;
};

} // namespace bh::gfx
