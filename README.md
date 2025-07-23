### Gravity simulation

<img width="1272" height="710" alt="exemplo" src="https://github.com/user-attachments/assets/0dd55d3b-3a2f-483f-8766-d3483764f554" />

---

* **Language**: C++17
* **Graphics API**: OpenGL 3.3+ (Core Profile)
* **Libraries**:
    * **GLFW**: For window creation and input management.
    * **GLEW**: For loading OpenGL extensions.
    * **GLM**: For vector and matrix mathematics.
* **Compiler**: g++ (MinGW-w64)
* **Environment**: MSYS2 (UCRT64)

---

The height y of any vertex on the grid is calculated using an inverted Gaussian function, which depends on its planar distance from the sphere.
The main equation for the height y of a vertex at ($$x_grid,z_grid$$) is:

$$y(x_{grid}, z_{grid}) = A \cdot e^{-s \cdot d^2} $$

The amplitude $A$ is not constant. It changes based on the sphere's height ($y\_{sphere}$) to simulate a stronger or weaker gravitational field.

Like this:

$$A\_{dynamic} = \\min(0, A\_{base} + (y\_{sphere} - y\_{base}) \\cdot k) $$

The $\\min(0, ...)$ function ensures the grid never deforms upwards (creating "hills").
