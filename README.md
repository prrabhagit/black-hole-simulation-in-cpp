# Schwarzschild Black Hole Renderer

Real-time, physically based Schwarzschild black hole ray tracer.
C++20 / OpenGL 4.6 Core / GLSL 460. Built in VS Code (or any CMake-aware IDE).

## What this is

Every pixel's ray is integrated through curved spacetime using RK4 on the
photon Binet equation, not screen-space UV distortion. See the project chat
log for the full physics derivation of each subsystem (Schwarzschild metric,
geodesics, photon sphere, lensing, accretion disk, Doppler beaming,
gravitational redshift, HDR/bloom, and the weak-field performance shortcut).

## One-time setup (do this BEFORE opening in VS Code / building)

1. **GLAD** (OpenGL loader) -- not fetched automatically, since it's a
   spec-generated loader rather than a git repo:
   - Go to https://glad.dav1d.de/
   - Language: C/C++, Specification: OpenGL, API gl: **4.6**, Profile: **Core**,
     check "Generate a loader"
   - Click Generate, download the zip, extract so you get:
     - `external/glad/include/glad/glad.h`
     - `external/glad/include/KHR/khrplatform.h`
     - `external/glad/src/glad.c`
   - (see `external/glad/README.md`)

2. **HDR starfield texture** -- put a real equirectangular (lat-long) HDR
   sky/starfield image at `textures/starfield.hdr`. Suggestions: ESO
   GigaGalaxy Zoom panoramas, NASA SVS Deep Star Maps, or any CC0 night-sky
   HDRI (e.g. Poly Haven) for initial testing.

Everything else (GLFW, GLM, Dear ImGui) is fetched automatically by CMake's
`FetchContent` on first configure -- you'll need network access the first
time you run CMake.

## Build

### VS Code
1. Install the **CMake Tools** extension (and the C/C++ extension) if you
   don't have them.
2. Open this folder in VS Code (`File > Open Folder...`).
3. CMake Tools should detect `CMakeLists.txt` and prompt you to configure --
   pick your installed compiler kit (MSVC, if on Windows).
4. `Ctrl+Shift+P` -> "CMake: Build" (or click Build in the status bar).
5. Run via the CMake Tools "Run" button, or directly:
   `build/bin/Release/blackhole_sim.exe` (Windows/MSVC multi-config) or
   `build/bin/blackhole_sim` (single-config generators).

### Command line
```
mkdir build && cd build
cmake .. -DCMAKE_BUILD_TYPE=Release
cmake --build . --config Release
./bin/Release/blackhole_sim.exe    # Windows
./bin/blackhole_sim                # Linux
```

## Controls

| Input | Effect |
|---|---|
| ImGui panel | Black hole mass, integration quality, disk params, camera mode/distance, debug overlays, exposure/bloom, validation suite |
| `1` / `2` | Switch orbit / free-fly camera mode |
| Orbit: left-drag / scroll | Rotate / zoom |
| Free-fly: right-drag / WASD / QE / Shift | Look / move / up-down / sprint |

## Project layout

```
CMakeLists.txt
src/            -- all headers (.hpp) and implementation (.cpp), paired side by side
                   e.g. Window.hpp/.cpp, Camera.hpp/.cpp, ShaderProgram.hpp/.cpp,
                   Application.hpp/.cpp (ties everything together), main.cpp (entry point)
shaders/        -- GLSL: fullscreen.vert + the 4 passes (raytrace/brightpass/blur/composite),
                   plus intersect.glsl, schwarzschild.glsl, geodesic.glsl (raytrace.frag is
                   the core physics, and #includes those three)
textures/       -- put starfield.hdr here
external/{stb,glad}/ -- vendored / user-generated dependencies
```

Headers and their `.cpp` live in the same `src/` folder next to each other,
so there's one place to look per class instead of hunting across
`include/` and `src/` subfolders.
