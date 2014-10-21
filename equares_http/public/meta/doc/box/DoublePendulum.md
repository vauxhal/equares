### Differential equations
Classical double pendulum consists of two point masses, $m_1$ and $m_2$, connected by rigid rods of lengths $l_1$ and $l_2$, with a joint at $m_2$. One end of first rod is fixed.
The point masses move in vertical plane, so the configuration of the system can be described by two angles, $\varphi$ and $\vartheta$, between rods and the vertical line &mdash; see
figure below. The system is placed in the constant gravity field, so that the gravity acceleration $g$ acts in the vertical direction.

![](/img/-/double-pendulum.png)

The equations of motion of the double pendulum are as follows.

$$\begin{array}{rcccrcl}
(m_1+m_2) l_1^2\ddot\varphi &+& m_2 l_1 l_2\left[\cos(\vartheta-\varphi)\ddot\vartheta - \sin(\vartheta-\varphi)\dot\vartheta^2\right] &+& g(m_1+m_2)\sin\varphi &=& 0,\\
m_2 l_2^2\ddot\vartheta &+& m_2 l_1 l_2\left[\cos(\vartheta-\varphi)\ddot\varphi - \sin(\vartheta-\varphi)\dot\varphi^2\right] &+& g m_2\sin\vartheta &=& 0.
\end{array}
$$

They can be obtained by considering ?[Lagrange equations of the second kind](http://en.wikipedia.org/wiki/Lagrangian_mechanics).

The double pendulum system is known to show quasi-periodic or chaotic behavior, depending on initial conditions.

The box obtains its parameters,  $l_1$, $m_1$, $l_2$, $m_2$, and $g$, from port **parameters** ([data hints](/doc#page/general-data) **l1**, **m1**, **l2**, **m2**, and **g** respectively),
and system state vector $[\varphi, \vartheta, \dot\varphi, \dot\vartheta, t]^T$ from port **state** (data hints **q1**, **q2**, **dq1**, **dq2**, **t**);
it sends ODE right hand side vector $[\dot\varphi, \dot\vartheta, \ddot\varphi, \ddot\vartheta]^T$ (data hints **dq1**, **dq2**, **d2q1**, and **d2q2**) to port **oderhs**.

### Data processing

The data processing for this box is the same as for any other ODE box, e.g., =[CxxOde](/doc#box/CxxOde).
It produces an output data frame with ODE right hand side as soon as system state comes to port **state**, provided
some parameters are already available at port **parameters** (if not, the processing is cancelled). Sending
data frames to port **parameters** *does not* cause ODE right hand side to be computed and data frame to be sent to port **oderhs**.

The **state** and **oderhs** ports are typically connected to the **rhsState** and **rhs** ports of an =[ODE solver](/doc#box/Rk4) box respectively,
and the **parameters** port can be connected, for example, to the output port of a [Param](/doc#box/Param) box.

### See also
- ?[This Wikipedia article](http://en.wikipedia.org/wiki/Double_pendulum)
- =[Double pendulum, Poincare map (interactive 2)](/editor?sim=double-pendulum-psec-interactive2) simulation
