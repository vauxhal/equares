### Differential equation
Simple pendulum is described by well known nonlinear equation
$$
  \ddot \varphi + \frac{g}{l}\sin \varphi = 0,
$$
where $\varphi$ is the angle between the pendulum and the vertical direction, $g$ is the gravitational acceleration, and $l$ is the length of the pendulum - see figure below.

![](/img/-/pendulum.png)

### Data processing
The box obtains its parameters, $l$ and $g$, from the **parameters** port, and its state variables, $\varphi$ and $\dot\varphi$, and the time, $t$,
from the **state** port (on that port, =[data hints](/doc#page/general-data) for state variables and time are **q**, **dq**, **t** respectively).

Once data on both ports is available, the box outputs ODE right hand side each time any of the inputs obtains a new data frame.
The right hand side $\dot\varphi$, $\ddot\varphi$ is then written to the **oderhs** port (on that port, data hints for elements are **dq** and **d2q** respectively).

The **state** and **oderhs** ports can be connected, for example, to the **rhsState** and **rhs** ports of an =[ODE solver](/doc#box/Rk4) box respectively,
and the **parameters** port can be connected to the output port of a [Param](/doc#box/Param) box.

See also the =[example of pendulum simulation](/editor?sim=z%2Fsimple-pendulum-1).
