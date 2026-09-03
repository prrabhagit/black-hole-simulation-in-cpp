cmake_minimum_required(VERSION 3.21)
project(blackhole_sim LANGUAGES CXX C)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

include(FetchContent)

# GLFW ------------------------------------------------------------------
FetchContent_Declare(
    glfw
    GIT_REPOSITORY https://github.com/glfw/glfw.git
    GIT_TAG        3.4
    GIT_SHALLOW    TRUE
)
set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_TESTS    OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_DOCS     OFF CACHE BOOL "" FORCE)
set(GLFW_INSTALL        OFF CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(glfw)

# GLM (header-only) -------------------------------------------------------
FetchContent_Declare(
    glm
    GIT_REPOSITORY https://github.com/g-truc/glm.git
    GIT_TAG        1.0.1
    GIT_SHALLOW    TRUE
)
FetchContent_MakeAvailable(glm)

# Dear ImGui --------------------------------------------------------------
FetchContent_Declare(
    imgui
    GIT_REPOSITORY https://github.com/ocornut/imgui.git
    GIT_TAG        v1.90.9
    GIT_SHALLOW    TRUE
)
FetchContent_MakeAvailable(imgui)

add_library(imgui STATIC
    ${imgui_SOURCE_DIR}/imgui.cpp
    ${imgui_SOURCE_DIR}/imgui_draw.cpp
    ${imgui_SOURCE_DIR}/imgui_tables.cpp
    ${imgui_SOURCE_DIR}/imgui_widgets.cpp
    ${imgui_SOURCE_DIR}/imgui_demo.cpp
    ${imgui_SOURCE_DIR}/backends/imgui_impl_glfw.cpp
    ${imgui_SOURCE_DIR}/backends/imgui_impl_opengl3.cpp
)
target_include_directories(imgui PUBLIC ${imgui_SOURCE_DIR} ${imgui_SOURCE_DIR}/backends)
target_link_libraries(imgui PUBLIC glfw)

# GLAD ----------------------------------------------------------------------
# NOT fetched from git -- generate once via https://glad.dav1d.de
# (Language=C/C++, API gl=4.6, Profile=Core, "Generate a loader") and place
# the output under external/glad/ (see external/glad/README.md).
add_library(glad STATIC external/glad/src/glad.c)
target_include_directories(glad PUBLIC external/glad/include)

# Main executable -----------------------------------------------------------
add_executable(blackhole_sim
    src/main.cpp
    src/Window.cpp
    src/GLDebug.cpp
    src/Camera.cpp
    src/ShaderProgram.cpp
    src/FullscreenTriangle.cpp
    src/UniformBuffer.cpp
    src/Texture2D.cpp
    src/Framebuffer.cpp
    src/GpuTimer.cpp
    src/stb_image_impl.cpp
    src/Application.cpp
)

target_include_directories(blackhole_sim PRIVATE src external/stb)

target_link_libraries(blackhole_sim PRIVATE glfw glad glm::glm imgui)

if (WIN32)
    target_link_libraries(blackhole_sim PRIVATE opengl32)
elseif(APPLE)
    target_link_libraries(blackhole_sim PRIVATE "-framework OpenGL")
else()
    target_link_libraries(blackhole_sim PRIVATE GL)
endif()

target_compile_options(blackhole_sim PRIVATE
    $<$<CXX_COMPILER_ID:MSVC>:/W4>
    $<$<NOT:$<CXX_COMPILER_ID:MSVC>>:-Wall -Wextra -Wpedantic>
)

add_custom_command(TARGET blackhole_sim POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
        ${CMAKE_SOURCE_DIR}/shaders $<TARGET_FILE_DIR:blackhole_sim>/shaders
    COMMAND ${CMAKE_COMMAND} -E copy_directory
        ${CMAKE_SOURCE_DIR}/textures $<TARGET_FILE_DIR:blackhole_sim>/textures
)
