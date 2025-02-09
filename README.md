# 2D Cloth Simulation using Euler Integration (SFML)

This project is a **2D cloth simulation** implemented using **Euler Integration** and rendered with the **Simple and Fast Multimedia Library (SFML)**. It demonstrates the behavior of a cloth-like material under the influence of gravity, wind, and user interaction.

---

## Features

- **Realistic Cloth Simulation**: Simulates the behavior of a 2D cloth using a grid of point masses connected by springs.
- **Euler Integration**: Uses Euler's method for numerical integration to update the positions and velocities of the cloth points.
- **Interactive Controls**:
  - Drag and move points of the cloth and collision object with the mouse.
  - Toggle gravity and wind effects on/off.
- **Rendering with SFML**: Utilizes SFML for rendering the cloth and handling user input.
- **Customizable Parameters**: Adjust parameters like gravity, wind force, and spring stiffness to observe different behaviors.

---

## Installation

### Prerequisites
- **SFML**: Ensure SFML is installed on your system. You can download it from the [official SFML website](https://www.sfml-dev.org/download.php).
- **C++ Compiler**: A C++ compiler that supports C++11 or later (e.g., GCC, Clang, or MSVC).

---

## Dependencies

- [SFML](https://www.sfml-dev.org/) (Simple and Fast Multimedia Library)

---

## Usage

- Right Mouse : Cut Spring (Cloth)<br/>
- Left Mouse : Move Collision body/cloth<br/>
- Middle Mouse : Show nodes/points<br/>
- Down Arrow : Turn Gravity on/off<br/>
- Up Arrow : Turn Spring Force on/off<br/>
- Left Arrow : Toggle between interaction of Collision body/Cloth nodes

---

## Implementation Details

### Cloth Model
- The cloth is modeled as a grid of point masses connected by structural springs.
- Each point mass is influenced by forces such as gravity, wind, and spring forces.

### Euler Integration
- Euler's method is used to update the positions and velocities of the point masses over time.

### Rendering
- SFML is used to render the cloth as a series of lines and circles representing the springs and point masses, respectively.
---
