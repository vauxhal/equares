### Overview
The box transmits _control data frames_ from port **control_in** to port **control_out**,
and _value data frames_ from port **value_in** to port **value_out**. Output frames are generated when a new control data frame comes to **control_in**,
and only if a data frame is available in **value_in**.

Ports **control_in** and **control_out** must have the same format; ports **value_in** and **value_out** must
have the same format too. The format for each pair of ports can be arbitrary.

The **Replicator** box helps to solve many different tasks. It can be used, for example, to synchronize data frames coming
to a =[Join](/doc#box/Join) box, but there are many other applications.

### Data processing

When a new value data frame comes to port **value_in**, nothing happens, and data processing in the box finishes.

When a new control data frame comes to port **control_in**, the following is done.
* If there is no data available at port **value_in**, the data processing is cancelled.
* Otherwise, two output frames are sent.
    * Firstly, the control data frame just received at port **control_in** is sent to port **control_out**.
    * Secondly, the last value data frame received at port **value_in** is sent to port **value_out**.

### See also
The **Replicator** box is used in many simulations, for example, consider the following examples.
* =[Interactive phase portrait, #5](/editor?sim=interactive-phase-portrait-5): the box **replicator** is used
  to solve initial value problem twice, first with positive integration step and then with negative step,
  so the resulting phase curve will pass through the point chosen by user.
* =[Interactive phase portrait, #6](/editor?sim=interactive-phase-portrait-6): one more replicator,
  **repl_h**, is added here, to join phase trajectory point with the current value of integration step.
* =[Chaos induced by logistic mapping](/editor?sim=logistic-mapping-chaos): the box **prepareOuput**
  is used before joining abscissa and ordinate of point into a single data frame.
* =[Mandelbrot set](/editor?sim=mandelbrot): the box **solverTrigger** is used to iterate FDE system
  many times with the same initial conditions, for each point in the space of FDE parameters.
* =[Mandelbrot set (enhanced, color-1)](/editor?sim=mandelbrot-enhanced-color-1): the box **r** is
  used before joining point coordinates with value determining its color.
