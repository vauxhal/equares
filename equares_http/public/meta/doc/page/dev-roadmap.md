# TODO: Hotlist
* core: Rename GridGenerator to Grid.
* web: less strict port equality check (hints are optional).
* core: validate number of interactive input values.
* docs: tutorial upload page.
* Correct equations for logistic mapping and Mandelbrot set.

# TODO: Roadmap

## Core functionality
### Common tasks
1. Ordinary differential equations
   1. Periodic solutions
   2. Phase portraits (autonomous systems with two phase variables)
2. Cellular automata
3. Simulation output
   1. Plots with axes, labels, marks, and captions
   2. Animations
   3. 3D data (e.g., curves)
4. Solvers
   1. ODE solver for conservative system (RK2 trapezoid rule)
   2. CVODE solver
   3. Nonlinear algebraic equations
   4. Linear algebra (LAPACK)
5. Parallelization
   1. Multicore
   2. GPU
   3. Cluster

### New boxes wanted
1. **Integrate** &mdash; similar to =[Differentiate](/doc#box/Differentiate), but different formula: should accumulate total
   sum of input data frames and output it as soon as a new data frame is received.
2. **Sum** &mdash; has two input ports, **in_1** and **in_2**, and one output port, **out**. All ports have the same format,
   output frame is computed from each two input frames (just like for =[Join](/doc#box/Join)). Output components are equal to
   the sums of components of both inputs.
3. **AffineTransform** &mdash; transforms input to output using an affine transformation specified
   by the **transformParam** parameter or (if connected) the **transform** port. The value of the **transformParam**
   parameter should be entered as text containing space-delimited elements of the transformation matrix, one line per matrix row;
   or, even better, a table should be provided by web interface to input the matrix.
4. **DistanceFilter** &mdash; similar to =[IntervalFilter](/doc#box/IntervalFilter), but curve parameter is computed from point coordinates
   according to **parameter**; the parameter must be an array of structures with fields **index**, **weight**.

### Box enhancement
Probably its reasonable to add functionality to some boxes, rather than achieve that functionality with chains of other boxes in
simulations. For example
1. The following parameters could be added to =[Canvas](/doc#box/Canvas) that control how incoming values are mapped to pixels:
   **pointSize** &mdash; controls how big should be the square of pixels representing each incoming point;
   **stroke** &mdash; boolean, ```true``` means that intermediate points are generated, so that lines are drawn instead of separate points.

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
