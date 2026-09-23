# Astral Engine

## Synopsis

Astral Engine is a 3D N-body orbital simulation framework written in modern C++ for numerically integrating the trajectories of multiple gravitationally interacting bodies. The physics core implements both Velocity Verlet and Runge–Kutta 4 (RK4) integration schemes, allowing a trade-off between computational cost and accuracy depending on the problem scale. All quantities (mass, position, velocity, gravitational parameter) are stored in SI units and propagated in double precision.

Beyond orbital dynamics, the engine now models rigid body behavior for simulated craft and objects, including collision detection and response, and torque-driven rotational physics. Orbit initialization and prediction tools allow bodies to be placed onto specified trajectories and their future paths to be computed and visualized ahead of simulation.

The engine is being developed as a general sandbox for orbital mechanics and GNC (Guidance, Navigation, and Control) research rather than as a fixed application.

## Rendering

Rendering is handled by a custom OpenGL pipeline that interfaces directly with the simulation state, built without dependence on an external game or graphics engine. Current rendering capabilities include:

* Textured model loading for spacecraft and celestial bodies
* Rayleigh and Mie scattering for physically-based, realistic planetary atmospheres
* Bloom and tonemapping post-processing
* Configurable rocket exhaust plume effects
* Ray tracing for rendering accurately scaled planet models without floating-point precision error, enabling true-to-scale distances and body sizes in the same scene

The codebase uses GLFW for context management, GLAD for OpenGL function loading, and ImGui for GUI purposes.

## Spacecraft Control

Astral Engine includes programmable spacecraft controllers driven by simulated gyroscope sensor data, allowing for closed-loop attitude control and programmable thrusting logic. Combined with the orbit initialization and prediction tools, this supports experimentation with guidance and control strategies against realistic rigid-body and orbital dynamics.

## Demonstration

Below is a run of the Sun–Earth–Moon system at true-to-scale sizes and distances, with the time step accelerated relative to real-time.

[astral_engine-1.webm](https://github.com/user-attachments/assets/f937bc72-c43d-4a87-bda2-e20ef40ddd34)

## In-Progress Features

* Ray-traced solar irradiance for realistic lighting and shadowing effects
* Atmospheric drag modeling for low-Earth orbit accuracy
* Real-time satellite tracking with TLE/ephemeris ingestion
* Real-time orbit characterization and parameter visualization tools

## Dependencies

- OpenGL
- GLFW
- GLAD
- ImGui
- AssImp

## How To Run

Simply clone the repo and run:

```
./build.sh && ./run.sh
```
