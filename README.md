```
# Simulated Black Hole Visualization — OpenGL (C++)

A real-time OpenGL-based C++ simulation that visualizes a simulated black hole and its gravitational capture behavior. The program renders a grid-based space where particles/rays entering the black hole’s influence region curve inward and get trapped inside the event horizon zone, producing a dynamic black hole effect.

This project focuses on graphics rendering, vector-field simulation, and physics-inspired attraction behavior using OpenGL.

---

## Project Overview

This project implements a simulated black hole effect using a gravity-style attraction field. Objects moving through the scene are influenced by the black hole’s pull based on distance. As they approach the core, their trajectories curve inward. Once they cross the event horizon radius, they are captured and removed from motion.

A reference grid is rendered in the background to make spatial distortion and motion easier to observe.

This is a visual simulation model, not a relativistic astrophysics solver.

---

## Features

- OpenGL real-time rendering
- Simulated black hole attraction field
- Inward curving trajectories near core
- Event horizon capture logic
- Grid-based spatial visualization
- Distance-based force strength
- Frame-by-frame motion updates
- Adjustable gravity and radius parameters
- Modular C++ structure

---

## Simulation Logic

Each moving element contains:

- Position vector
- Direction vector
- Speed

Per-frame update:

1. Compute vector toward black hole center
2. Measure distance
3. Compute attraction force based on distance
4. Adjust movement direction toward center
5. Normalize direction
6. Update position
7. If inside event horizon → capture/remove

---

## Core Motion Formula

```

to_center = normalize(bh_pos - obj_pos)
force = gravity_strength / (distance * distance)

direction = normalize(direction + to_center * force)
position += direction * speed

```

---

## Rendering Pipeline

- Initialize OpenGL context
- Setup window and viewport
- Configure orthographic or perspective projection
- Draw background grid
- Draw black hole core
- Draw moving rays/particles
- Update simulation each frame
- Swap buffers

---

## Controls (Adjust to match your implementation)

```

R → Reset simulation
Arrow Keys → Move black hole center

* / - → Adjust gravity strength
  G → Toggle grid
  Space → Pause/resume

```

---

## Configurable Parameters

```

gravity_strength
event_horizon_radius
object_speed
spawn_rate
max_objects
grid_spacing
time_step

```

---

## Tech Stack

- C++
- OpenGL
- GLFW / GLUT / SDL (whichever you used)
- GLAD / GLEW (if used)
- STL math utilities

Update based on your actual setup.

---

## Build Instructions (Example)

### Linux / Mac

```

g++ main.cpp -lGL -lglfw -lGLEW -o blackhole_sim
./blackhole_sim

```

### Windows (MinGW example)

```

g++ main.cpp -lopengl32 -lglfw3 -lglew32 -o blackhole_sim.exe

```

Adjust library flags as needed.

---

## Learning Outcomes

This project demonstrates:

- OpenGL rendering workflow
- Real-time simulation loops
- Vector math and normalization
- Distance-field force systems
- Capture radius logic
- Visual physics-style effects
- Parameter-driven animation

---

## Possible Enhancements

- Accretion disk visualization
- Particle glow shaders
- Distortion shaders
- 3D camera mode
- UI parameter sliders
- GPU particle acceleration
- Multi-black-hole simulation

---

## Demo Result

Expected output:

- Objects curve inward near the core
- Strong attraction near center
- Capture inside event horizon
- Clear grid reference for spatial motion

---

## License

Add your preferred license (MIT recommended).

---

## Author

Project: Simulated Black Hole Visualization  
Language: C++ with OpenGL  
Category: Graphics Simulation / Physics-Inspired Visualization

```
