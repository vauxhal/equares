### Overview
The box glues together each two data frames coming to input ports **in_1** and **in_2**. Both input data frames must contain 1D data,
and the output data frame is also 1D.

Suppose that the input data frame at port **in_1** has $n_1$ elements $x^{in,1}_0,\ldots x^{in,1}_{n_1-1}$, and
the input data frame at port **in_2** has $n_2$ elements $x^{in,2}_0,\ldots x^{in,2}_{n_2-1}$.
Then the output data frame consists of all elements of data frame at port **in_1**, followed by all elements of data frame at port **in_2**:
$x^{in,1}_0,\ldots x^{in,1}_{n_1-1}, x^{in,2}_0,\ldots x^{in,2}_{n_2-1}$, so it has $n_1+n_2$ elements.

### Data processing
The box has two boolean state variables, $s_1$ and $s_2$, indicating that an unprocessied data frame is pending at input ports **in_1** and **in_2** respectively. The value of ```true``` means
that an input data frame has been received but has not been processed so far. The value of ```false``` means that there were no data frames at all, or the last received
input data frame has already been processed.

When an input data frame comes to port **in_1** or **in_2**, the value of state variable $s_1$ or $s_2$ respectively *must be* ```false```. Otherwise, simulation stops with
the error message saying "Join box overflow". Then, the state variable is set to ```true```. After that, if the other state variable ($s_2$ or $s_1$ respectively) is ```false```,
the processing finishes (waiting for a data frame at the other input port).
If both state variables $s_1$ and $s_2$ are ```true```, the box resets them to ```false``` and generates one output frame and sends it to port ```out```.

The logics of the box ensures that $k$-th output data frame at port **out** is generated from $k$-th data frame at input port **in_1** and $k$-th data frame at input port **in_2**.

### See also
- The =[Replicator](/doc#box/Replicator) box is often used in combination with the **Join** box to ensure no overflow error.
- The following simulations demonstrate the usage of the **Join** box:
    - =[Chaos induced by logistic mapping](/editor?sim=logistic-mapping-chaos);
    - =[Simple pendulum, phase volume evolution](/editor?sim=phase-volume-evolution);
    - =[Interactive phase portrait, #6](/editor?sim=interactive-phase-portrait-6) and step 6 of =[this tutorial](/doc#tut/-/interactive-phase-portrait);
    - =[Mandelbrot set (enhanced, color-1)](/editor?sim=mandelbrot-enhanced-color-1), =[Mandelbrot set (enhanced, color-2)](/editor?sim=mandelbrot-enhanced-color-2).
