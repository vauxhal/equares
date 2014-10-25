### Overview
The =[Rk4](/doc#box/Rk4) box performing numerical solution of initial value problem for ODE systems
requires that the time integration step, $h$, the number of iterations, $n$, and output control parameter, $n_{out}$, be specified
as its parameters coming to its **parameters** port.

However, it is sometimes more convenient to specify the duration of integration interval, $T$, explicitly, and have some other parameter
($h$ or $n$) adjusted such that $nh=T$. Or it might be desirable to specify one parameter independently from other parameters.
This is exactly what this box does. It obtains an input combination of parameters in three of four input ports
* **h** &mdash; for desired time integration step, $h^{in}$
* **n** &mdash; for desired number of iterations, $n^{in}$
* **T** &mdash; for intefration period duration, $T$
* **nout** &mdash; for output control parameter, $n_{out}$

and writes output parameters ${\bf p}^{RK4}=[h^{RK4},n^{RK4},n_{out}]^T$ to the output port **rk4param** as soon as any
new input data frame is obtained.

The box allows user to specify the following combinations of parameters:
* $h^{in},\ n^{in},\ n_{out}$ &mdash; these are the same parameters that =[Rk4](/doc#box/Rk4) accepts. The reason for using this box in this case
  is the ability to change just one parameter and keep other parameters intact. The solver will obtain parameters
  $h^{RK4}=h^{in}$, $n^{RK4}=n^{in}$.
* $h^{in},\ T,\ n_{out}$ &mdash; this combination is convenient if $T$ needs to be specified. In this case, the solver will obtain
  $n^{RK4}=\left\lfloor T \ /\ h^{in}\right\rfloor$, $h^{RK4}=T \ /\ n^{RK4}$.

### Data processing
The box expects that input ports connected to something else make one of the following combinations:
* **h**, **n**, **nout** or
* **h**, **T**, **nout**.

The box deduces what is specified from which input ports are connected.

Once a data frame comes to an input port, the following happens.
* If there is no data available on one of other connected input ports, the processing is cancelled.
* Otherwise, ${\bf p}^{RK4}$ is computed and sent to port **rk4param**. In that computation, last values
  obtained at each of input ports are used.

### See also
* =[Rk4](/doc#box/Rk4)
* The following simulations show examples of usage of the **Rk4ParamAdjust** box.
    * =[Ince-Strutt diagram](/editor?sim=mathieu-stability)
    * =[Simple pendulum, phase volume evolution](/editor?sim=phase-volume-evolution)
