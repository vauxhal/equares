### Overview

The Mathieu equation is a linear equation with periodic coefficients:
$$
\ddot q + \left[\lambda - 2\gamma\cos(2q)\right]q = 0,
$$
where $\lambda$ and $\gamma$ are parameters.
The equation is well known due to the parametric resonance phenomenon it demonstrates.

The box obtains its parameters, $\lambda$ and $\gamma$, from port **parameters** ([data hints](/doc#page/general-data) **lambda** and **gamma** respectively),
and system state vector $[q, \dot q, t]^T$ from port **state** (data hints **q**, **dq**, **t**);
it sends ODE right hand side vector $[\dot q, \ddot q]^T$ (data hints **dq** and **d2q**) to port **oderhs**.

### Data processing

The data processing for this box is basically the same as for any other ODE box, e.g., =[CxxOde](/doc#box/CxxOde).
It produces an output data frame with ODE right hand side as soon as system state comes to port **state**, provided
some parameters are already available at port **parameters** (if not, the processing is cancelled). Sending
data frames to port **parameters** *does not* cause ODE right hand side to be computed and data frame to be sent to port **oderhs**.

The **state** and **oderhs** ports are typically connected to the **rhsState** and **rhs** ports of an =[ODE solver](/doc#box/Rk4) box respectively,
and the **parameters** port can be connected, for example, to the output port of a [Param](/doc#box/Param) box.

In addition to the standard ODE box behavior, this box has one more output port, **period**. A data frame is always available at this port;
it contains one element, which is the period of time dependency of equation coefficients (it equals $\pi$).
Notice that the box never causes any other boxes attached to this port to start processing that data. Instead,
they can use the period when they need it.

### See also

- =[Ince-Strutt diagram](/editor?sim=mathieu-stability) simulation
- The =[Rk4](/doc#box/Rk4) box is currently the only ODE solver that can be connected to ports **state** and **oderhs**
