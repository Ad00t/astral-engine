# Astral Engine

## Synopsis

Astral Engine is a 3D N-body orbital simulation framework written in modern C++ designed for numerically integrating the trajectories of multiple gravitationally interacting bodies. The physics core implements both Velocity Verlet and Runge–Kutta 4 (RK4) integration schemes, allowing a trade-off between computational cost and accuracy depending on the problem scale. All quantities (mass, position, velocity, gravitational parameter) are stored in SI units and propagated in double precision to reduce round-off error during long integrations.

Rendering is handled by a custom OpenGL pipeline that interfaces directly with the simulation state. The pipeline includes GLSL shaders for vertex and fragment operations and is structured to support large numbers of dynamic objects without depending on an external game or graphics engine. The codebase uses GLFW for context management, GLAD for OpenGL function loading, and ImGui for GUI purposes.

The engine is being developed as a general sandbox for orbital mechanics and GNC research rather than as a fixed application.

## Demonstration

Below is a high-speed run of the Sun–Earth–Moon system at true-to-scale sizes and distances. For demonstration, the time step has been increased up to 100,000× real-time, which exposes numerical instability (the “glitching” you’ll notice). Testing new integration and error-control methods to mitigate this instability is an active development area.

[astral_engine-1.webm](https://github.com/user-attachments/assets/f937bc72-c43d-4a87-bda2-e20ef40ddd34)

## In-Progress Features

* GPU-accelerated shaders for detailed texture mapping of celestial bodies

* Ray-traced solar irradiance for realistic lighting and shadowing effects

* Atmospheric drag modeling for low-Earth orbit accuracy

* Real-time satellite tracking with TLE/ephemeris ingestion

* Real-time orbit characterization and parameter visualization tools

## Dependencies

 - OpenGL
 - GLFW
 - GLAD
 - ImGui

## How To Run

Simply clone the repo and run:

```
./build.sh && ./run.sh
```

