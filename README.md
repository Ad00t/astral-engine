# Astral Engine

## Synopsis

Astral Engine is a 3D N-body orbital simulation framework written in modern C++ for numerically integrating the trajectories of multiple gravitationally interacting bodies. The physics core implements Velocity Verlet and Runge–Kutta 4 (RK4) integration schemes, allowing a trade-off between computational cost and accuracy depending on the problem scale. All quantities (mass, position, velocity, gravitational parameter) are stored in SI units and propagated in double precision.

Beyond orbital dynamics, the engine now models rigid body behavior for simulated craft and objects, including collision detection and response, and torque-driven rotational physics. Orbit initialization and prediction tools allow bodies to be placed onto specified trajectories and their future paths to be computed and visualized ahead of simulation.

The engine is being developed as a general sandbox for orbital mechanics and GNC (Guidance, Navigation, and Control) research rather than as a fixed application.

## Demonstration

[astral-engine-demo-2.mp4](https://github.com/user-attachments/assets/ec9ce5c7-26cd-4210-801f-feff483dcd39)

## Rendering

Rendering is handled by a custom OpenGL pipeline that interfaces directly with the simulation state, built without dependence on an external game or graphics engine. Current rendering capabilities include:

* Textured model loading for spacecraft and celestial bodies
* Rayleigh and Mie scattering for physically-based, realistic planetary atmospheres
* Bloom and HDR tonemapping post-processing
* Configurable rocket exhaust plume effects
* Ray tracing for rendering accurately scaled planet models without floating-point precision error, enabling true-to-scale distances and body sizes in the same scene

The codebase uses GLFW for context management, GLAD for OpenGL function loading, AssImp for 3D model loading, and ImGui for GUI purposes.

## Spacecraft Control

Astral Engine includes programmable spacecraft controllers driven by simulated gyroscope sensor data, allowing for closed-loop attitude control and programmable thrusting logic. Combined with the orbit initialization and prediction tools, this supports experimentation with guidance and control strategies against realistic rigid-body and orbital dynamics.

## In-Progress Features

* Improved ray-traced lighting system to enable solar power simulation, thermal cycling
* Atmospheric drag modeling for low-planetary orbit accuracy
* Real-time satellite tracking with TLE/ephemeris ingestion
* Higher fidelity propulsion simulation including thrust vector control, thrust curves, choked flow in converging-diverging nozzles, electric & cold gas engines
* More planetary bodies & moons for simulation of complex mission trajectories

## Dependencies

- OpenGL
- GLFW
- GLAD
- ImGui
- AssImp

## How To Run

This project is built via CMake but scripts are included to make the process easier.
Simply clone the repo and run:

```
./build.sh && ./run.sh
```
