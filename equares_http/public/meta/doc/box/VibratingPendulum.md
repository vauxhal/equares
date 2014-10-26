Asimple pendulum is considered, differing from the classical simple pendulum (see the =[Pendulum](/doc#box/Pendulum) box) only in that
the support is moving in the vertical direction, so its vertical displacement is
$$
u(t) = a\sin(\omega t),
$$
where $a$ is the amplitude, $\omega$ is the frequency, and $t$ is the time.

The equation of motion is as follows.

$$
\ddot\varphi + \frac{1}{l}\left[ - a\omega^2\sin(\omega t) + g \right]\sin\varphi = 0
$$

![](/img/-/vibrating-pendulum.png)

The box obtains its parameters,  $l$, $g$, $a$, and $\omega$, from port **parameters** ([data hints](/doc#page/general-data) **l**, **g**, **a**, **omega** respectively),
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

* The linearization of equation of motion and further change of time variable leads to Mathieu equation &mdash; see the =[Mathieu](/doc#box/Mathieu) box
* The =[Vibrating pendulum, Poincare map](/editor?sim=vibrating-pendulum-psec) simulation
* The =[Rk4](/doc#box/Rk4) box is currently the only ODE solver that can be connected to ports **state** and **oderhs**
