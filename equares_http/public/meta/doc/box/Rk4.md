### Overview
The box is designed to numerically solve initial value problem for systems of ordinary differential equations (ODE).
An ODE system should be connected to ports **rhsState**, **rhs**, so that
the solver can evaluate the right hand side of the system by writing the current state to **rhsState** and then read the right hand side from **rhs**.

The process of numerical solution starts as soon as any data frame comes to the **initState** port. At this point, some data frame must be available in the **parameters**
port (see below); otherwise, the processing is cancelled. The data on the **initState** port is the state of the ODE system from which the numerical solution starts.

As the solver generates new points of phase trajectory of the ODE system,
it writes them to port **nextState**; these data frames can be utilized by other parts of simulation workflow.
Notice that the initial state itself _will not_ be sent to that port.

The iterations stop when all requested iterations are done, or when the **stop** port receives any data frame. At this point, the solver sends empty data frame to its **finish** port
to let the rest of the workflow know that the iterations have finished.

The number of iterations and other parameters are specified by data frames coming to port **parameters**. These parameters are as follows:
* $h$ &mdash; the value of integration step;
* $n$ &mdash; the total number of iterations to perform;
* $n_{out}$ &mdash; determines how many iterations, should be done between two consequent frames sent to port **nextState**:
  1 means one output per iteration, 2 means one output per two iterations, and so on.

The **Rk4** box solves ODE initial value problem by using the explicit ?[Runge-Kutta scheme](http://en.wikipedia.org/wiki/Runge%E2%80%93Kutta_methods) of 4-th order.
Assume that ${\bf y}=[y_1,\ldots y_n]^T$ is the state of an ODE system of $n$-th order, and the equations are formulated in the normal form, so that
the time derivative of ${\bf y}$ is specified as a function of time $t$ and state ${\bf y}$:
$$
\dot{\bf y} = {\bf f}(t, {\bf y}).
$$
Now assume that ${\bf y}^i=[y^i_1,\ldots y^i_n]^T$ is the state of the system at $i$-th step, which corresponds to the time value $t=t^i$.
Then the next state ${\bf y}^{i+1}$, corresponding to the time value $t=t^{i+1}=t^i+h$ is obtained as follows.
$$\begin{array}{rcl}
{\bf y}^{i+1} &=& {\bf y}^i + \frac{h}{6}\left[{\bf k}^i_1 + 2 {\bf k}^i_2 + 2 {\bf k}^i_3 + {\bf k}^i_4\right],\vphantom{=}\\
{\bf k}^i_1 &=& {\bf f}\left(t,\ {\bf y}^i\right),\vphantom{=}\\
{\bf k}^i_2 &=& {\bf f}\left(t+\frac{h}{2},\ {\bf y}^i+\frac{h}{2}{\bf k}^i_1\right),\vphantom{=}\\
{\bf k}^i_3 &=& {\bf f}\left(t+\frac{h}{2},\ {\bf y}^i+\frac{h}{2}{\bf k}^i_2\right),\vphantom{=}\\
{\bf k}^i_4 &=& {\bf f}\left(t+h,\ {\bf y}^i+h{\bf k}^i_3\right).\vphantom{=}
\end{array}
$$

Notice that data frames describing system state include the time, so they are ${\bf x}=[y_1,\ldots y_n, t]^T=\left[\begin{array}{c}{\bf y}\\ t\end{array}\right]$.

### Data proceessing
Let us describe the data processing in more detail.
* When a data frame comes to port **parameters**, it specifies solver parameters $h$, $n$, and $n_{out}$ for one or more further solution processes.
* When a data frame comes to port **initState** it contains the initial state ${\bf x}^0$ of the ODE system. The solution process starts then.
    * Solver parameters are read from port **parameters**. If no data is available there, the data processing is cancelled. Notice the following about the solver parameters.
        * If $n$ is zero, it is set to the maximum integer number, so the number of iterations becomes practically infinite.
        * If $n_{out}$ is zero or negative, only the last system state, ${\bf x}^n$, will be written to port **nextState**.
    * The ability of ODE system to compute next state is checked: the initial state ${\bf x}^0$ is sent to port **rhsState** (which itself can cancel processing
      if the ODE is not ready e.g. because it hasn not received its parameters yet); then, if processing is not cancelled by ODE, the data available in
      port **rhs** is taken. If there is no data there, the processing is cancelled. Otherwise, the data is assumed to be ${\bf x}^i$, $i=1$, and the process
      continues. Notice that here and below $i$ is the iteration number.
    * Variable $s$ responsible for output to the **nextState** port is set to $n_{out}$, if it is positive, or to $n$ otherwise.
    * The solution loop is entered. The loop has $n$ iterations but can break earlier if any data frame will come to the **stop** port.
      At each iteration of the loop, the following is done.
        * Next system state ${\bf x}^{i+1}$ is computed accordingly to the RK4 scheme. ODE right hand side is evaluated by sending ${\bf x}^i$ to port **rhsState**,
          followed by reading ${\bf f}(t^i, {\bf y}^i)$ from port **rhs**.
        * Variable $s$ is decreased by one, and if it reaches zero, the state vector ${\bf x}^{i+1}$ is sent to port **nextState**; in this case, $s$ is set to $n_{out}$ again.
          Notice that sending data to **nextState** can cancel the processing, if some other box is not ready.
        * Iteration number $i$ is increased by one.
    * Empty data frame is sent to port **finish**.
* When a data frame comes to the **stop** port, the termination flag is set, which causes the currently running solution to exit the iteration loop.
  This has no effect if there is no solution process currently running.

Note that ports **rhs** and **rhsState** must not be connected to anything other than an ODE system. As follows from the data processing algorithm above,
the RK4 solver will make the following assumptions about these ports:
* When a data frame is sent to **rhsState**, one of two things can happen:
    * exactly one data frame comes to **rhs**;
    * the data processing is cancelled.
* No data frames come to **rhs** if nothing is sent to **rhsState**.

### See also
* The =[CxxOde](/doc#box/CxxOde) box implements logics for an ODE system. Concrete systems can be provided by using =[snippets](/doc#page/general-snippets).
* There are several hard-coded ODE systems, see the following boxes:
    * =[DoublePendulum](/doc#box/DoublePendulum)
    * =[Mathieu](/doc#box/Mathieu)
    * =[Pendulum](/doc#box/Pendulum)
    * =[VibratingPendulum](/doc#box/VibratingPendulum)
* Example simulations
    * =[Simple pendulum, phase trajectory](/editor?sim=simple-pendulum-1) (simplest example)
    * =[Interactive phase portrait, #6](/editor?sim=interactive-phase-portrait-6)
    * =[Double pendulum, Poincare map (interactive 2)](/editor?sim=double-pendulum-psec-interactive2)
    * =[Forced Duffing equation, Poincare map](/editor?sim=forced-duffing-psec)
    * =[Vibrating pendulum, Poincare map](/editor?sim=vibrating-pendulum-psec)
    * =[Ince-Strutt diagram](/editor?sim=mathieu-stability)
    * =[Simple pendulum, phase volume evolution](/editor?sim=phase-volume-evolution)
    * =[Lorenz attractor](/editor?sim=lorenz-attractor)
* The =[Rk4ParamAdjust](/doc#box/Rk4ParamAdjust) box allows to adjust number of iterations and integration step so that the specified intefration time is matched.
* The =[FdeIterator](/doc#box/FdeIterator) box is very similar to the RK4 solver but is designed to numerically solve finite difference equations (FDE).
