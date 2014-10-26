### Overview

Simple pendulum is described by well known nonlinear equation
$$
  \ddot \varphi + \frac{g}{l}\sin \varphi = 0,
$$
where $\varphi$ is the angle between the pendulum and the vertical direction, $g$ is the gravitational acceleration, and $l$ is the length of the pendulum &mdash; see figure below.

![](/img/-/pendulum.png)

The box obtains its parameters,  $l$ and $g$, from port **parameters** ([data hints](/doc#page/general-data) **l** and **g** respectively),
and system state vector $[\varphi, \dot\varphi, t]^T$ from port **state** (data hints **q**, **dq**, **t**);
it sends ODE right hand side vector $[\dot\varphi, \ddot\varphi]^T$ (data hints **dq** and **d2q**) to port **oderhs**.

### Data processing

The data processing for this box is the same as for any other ODE box, e.g., =[CxxOde](/doc#box/CxxOde).
It produces an output data frame with ODE right hand side as soon as system state comes to port **state**, provided
some parameters are already available at port **parameters** (if not, the processing is cancelled). Sending
data frames to port **parameters** *does not* cause ODE right hand side to be computed and data frame to be sent to port **oderhs**.

The **state** and **oderhs** ports are typically connected to the **rhsState** and **rhs** ports of an =[ODE solver](/doc#box/Rk4) box respectively,
and the **parameters** port can be connected, for example, to the output port of a [Param](/doc#box/Param) box.

### See also

- The =[example of pendulum simulation](/editor?sim=simple-pendulum-1)
- The =[Rk4](/doc#box/Rk4) box is currently the only ODE solver that can be connected to ports **state** and **oderhs**
