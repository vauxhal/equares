### Differential equations
Classical double pendulum consists of two point masses, $m_1$ and $m_2$, connected by rigid rods of lengths $l_1$ and $l_2$, with a joint at $m_2$. One end of first rod is fixed.
The point masses move in vertical plane, so the configuration of the system can be described by two angles, $\varphi$ and $\psi$, between rods and the vertical line &mdash; see
figure below. The system is placed in the constant gravity field, so that the gravity acceleration $g$ acts in the vertical direction.

![](/img/-/double-pendulum.png)

The equations of motion of the double pendulum are as follows.

$$\begin{array}{rcccrcl}
(m_1+m_2) l_1^2\ddot\varphi &+& m_2 l_1 l_2\left[\cos(\psi-\varphi)\ddot\psi - \sin(\psi-\varphi)\dot\psi^2\right] &+& g(m_1+m_2)\sin\varphi &=& 0,\\
m_2 l_2^2\ddot\psi &+& m_2 l_1 l_2\left[\cos(\psi-\varphi)\ddot\varphi - \sin(\psi-\varphi)\dot\varphi^2\right] &+& g m_2\sin\psi &=& 0.
\end{array}
$$

They can be obtained by considering ?[Lagrange equations of the second kind](http://en.wikipedia.org/wiki/Lagrangian_mechanics).

The double pendulum system is known to show quasi-periodic or chaotic behavior, depending on initial conditions.

### Data processing
The box obtains its parameters, $l_1$, $m_1$, $l_2$, $m_2$, and $g$, from the **parameters** port, and its state variables, $\varphi$, $\psi$, $\dot\varphi$, $\dot\psi$, and the time, $t$,
from the **state** port (on that port, =[data hints](/doc#page/general-data) for state variables and time are **q1**, **q2**, **dq1**, **dq2**, **t** respectively).

Once data on both ports is available, the box outputs ODE right hand side each time any of the inputs obtains a new data frame.
The right hand side $\dot\varphi$, $\dot\psi$, $\ddot\varphi$ $\ddot\psi$ is then written to the **oderhs** port (on that port, data hints for elements are **dq1**, **dq2**, **d2q1**, and **d2q2** respectively).

The **state** and **oderhs** ports can be connected, for example, to the **rhsState** and **rhs** ports of an =[ODE solver](/doc#box/Rk4) box respectively,
and the **parameters** port can be connected to the output port of a [Param](/doc#box/Param) box.

### See also
- ?[This Wikipedia article](http://en.wikipedia.org/wiki/Double_pendulum)
- =[This simulation](/editor?sim=double-pendulum-psec-interactive2)
