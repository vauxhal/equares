# Hotlist
* core: Rename GridGenerator to Grid.
* docs: Add section about port activation.
* core, web: Indicate ports that are activators.
* all: License file, add license text to all source files.
* web: less strict port equality check (hints are optional).
* docs: disclaimer; unstable format warning.
* core: validate number of interactive input values.
* docs: tutorial, pitfalls.
* web: split examples to basic and advanced.
* web: collections of ODE, FDE, or whatever.
---
* Correct equations for logistic mapping and Mandelbrot set.
* Canvas box update when withInputValue property changs.
* User profile page.
* Email address validation.
* Forgotten password.
* Change password.

# Roadmap / TODO

## Core functionality
### Common tasks
1. Ordinary differential equations
   1. ~~Stability diagrams (linear systems with periodic coefficients)~~
   2. Periodic solutions
   3. Phase portraits (autonomous systems with two phase variables)
2. Finite difference equations
   1. ~~Stable periodic solutions~~
   2. ~~Bifurcation diagrams~~
   3. ~~Fractals~~
3. Cellular automata
4. Simulation output
   1. Plots with various captions
   2. Animations
5. Solvers
   1. ODE solver for conservative system (RK2 trapezoid rule)
   2. CVODE solver
   3. Nonlinear algebraic equations
   4. Linear algebra (LAPACK)
6. Parallelization
   1. Multicore
   2. GPU
   3. Cluster

### Boxes
1. GridGenerator (with output port containing the description of the grid of frames)
2. Canvas with additional input port containing output data format
3. ~~Linear ODE with periodic coefficients~~
4. ~~Stability tester for linear ODE with periodic coefficients~~
5. Phase portrait generator
6. ~~FDE (finite difference equation) of first order~~
7. ~~FDE runner (on output we know if the solution is periodic, limited, or growing)~~
9. Input transformer
10. Canvas painter
11. ~~ParamArray~~
11. ~~Interpolator~~

## Web interface
1. Email notifications
   * Forgotten password
   * Updates
2. User feedback
   * Simulations
   * How to
   * Feature requests
   * Bug reports
   * Feedback form
3. View for curves in 3D
4. View for 2D and 3D projections of curve in nD
5. Simulation scheme visualizer
   * Move ports
   * Allow curved links

## Code
1. Refactor web interface
2. ?[Doxygen](http://www.doxygen.org) comments for core
3. Make functionality of equares-box.js and partially of scheme-editor.js available in equares_core
   in order to make it possible to run JSON-files stored in the DB. This will also fix potentially
   buggy behavior of buildDirs().
4. Unit tests
