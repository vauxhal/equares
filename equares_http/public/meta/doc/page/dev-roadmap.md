# Hotlist
* core: Rename GridGenerator to Grid.
* docs: Add section about port activation.
* core, web: Indicate ports that are activators.
* web: less strict port equality check (hints are optional).
* docs: disclaimer; unstable format warning.
* core: validate number of interactive input values.
* docs: tutorial, pitfalls.
* web: split examples to basic and advanced.
* Correct equations for logistic mapping and Mandelbrot set.
* Canvas box update when withInputValue property changes.

# Roadmap / TODO

## Core functionality
### Common tasks
1. Ordinary differential equations
   1. Periodic solutions
   2. Phase portraits (autonomous systems with two phase variables)
2. Cellular automata
3. Simulation output
   1. Plots with various captions
   2. Animations
4. Solvers
   1. ODE solver for conservative system (RK2 trapezoid rule)
   2. CVODE solver
   3. Nonlinear algebraic equations
   4. Linear algebra (LAPACK)
5. Parallelization
   1. Multicore
   2. GPU
   3. Cluster

### Boxes
1. GridGenerator (with output port containing the description of the grid of frames)
2. Canvas with additional input port containing output data format
3. Phase portrait generator
4. Input transformer
5. Canvas painter

## Web interface
1. View for curves in 3D
2. View for 2D and 3D projections of curve in nD
3. Simulation scheme visualizer
   * Allow curved links
   * Automatic scheme layout (something similar to ?[this](http://bl.ocks.org/d3noob/5141278))

## Code
1. Refactor web interface
2. ?[Doxygen](http://www.doxygen.org) comments for core
3. Make functionality of equares-box.js and partially of scheme-editor.js available in equares_core
   in order to make it possible to run JSON-files stored in the DB. This will also fix potentially
   buggy behavior of buildDirs().
4. Unit tests
