### Overview
The box is designed to iterate (or numerically solve) systems of finite difference equations. A finite difference equation (FDE) system should be connected to ports **fdeOut**, **fdeIn**, so that
the solver can evaluate each next state of the system by writing the current state to **fdeOut** and then read next state from **fdeIn**.

The process of numerical solution starts as soon as any data frame comes to the **initState** port. At this point, some data frame must be available in the **parameters**
port (see below); otherwise, the processing is cancelled. The data on the **initState** port is the state of the FDE system from which iterations start.

As the solver iterates the FDE system, it writes its sequential states to port **nextState**; these data frames can be utilized by other parts of simulation workflow.
Notice that the initial state itself will be sent to that port if **nskip**=0 and **nout**=1 (see below). The last state _will not_ be sent to port **nextState**.

The iterations stop when all requested iterations are done, or when the **stop** port receives any data frame. At this point, the solver sends empty data frame to its **finish** port
to let the rest of the workflow know that the iterations have finished.

The number of iterations and the output control parameters are specified by data frames coming to port **parameters**. These parameters are as follows:
* **n** &mdash; the total number of iterations to perform;
* **nskip** &mdash; determines how many initial iterations to perform before anything is sent to port **nextState**;
* **nout** &mdash; determines how many iterations should be done between two consequent frames sent to port **nextState**:
  1 means one output per iteration, 2 means one output per two iterations, and so on.

### Data proceessing
Let us describe the data processing in more detail.
* When a data frame comes to port **parameters**, it specifies solver parameters **n**, **nskip**, and **nout** for one or more further solution processes.
* When a data frame comes to port **initState** it contains the initial state $x_0$ of the FDE system. The solution process starts then.
    * Solver parameters are read from port **parameters**. If no data is available there, the data processing is cancelled. Notice the following about the solver parameters.
        * If **n** is zero, it is set to the maximum integer number, so the number of iterations becomes practically infinite.
        * If **nout** is zero or negative, only the last system state, $x_{n}$, will be written to port **nextState**.
    * The ability of FDE system to compute next state is checked: the initial state $x_0$ is sent to port **fdeOut** (which itself can cancel processing
      if the FDE is not ready e.g. because it hasn not received its parameters yet); then, if processing is not cancelled by FDE, the data available in
      port **fdeIn** is taken. If there is no data there, the processing is cancelled. Otherwise, the data is assumed to be $x_k$, $k=1$, and the process
      continues. Notice that here and below $k$ is the iteration number.
    * Variable $s$ responsible for output to the **nextState** port is set to **nout**, if it is positive, or to **n &ndash; nskip** otherwise.
    * The solution loop is entered. The loop has **n** iterations but can break earlier if any data frame will come to the **stop** port.
      At each iteration of the loop, the following is done.
        * If $k$ is greater than or equal to **nskip**, then $x_k$ is probably output to the **nextState** port. To decide whether to output or not,
          $s$ is decreased by one, and if it reaches zero, the output is done; in this case, $s$ is set to **nout** again.
          Notice that sending data to **nextState** can cancel the processing, if some other box is not ready.
        * Next system state $x_{k+1}$ is evaluated by sending $x_k$ to port **fdeOut**, followed by reading from port **fdeIn**.
        * Iteration number $k$ is increased by one.
    * Empty data frame is sent to port **finish**.
* When a data frame comes to the **stop** port, the termination flag is set, which causes the currently running solution to exit the iteration loop.
  This has no effect if there is no solution process currently running.

Note that ports **fdeIn** and **fdeOut** must not be connected to anything other than an FDE system. As follows from the data processing algorithm above,
the FDE iterator will make the following assumptions about these ports:
* When a data frame is sent to **fdeOut**, one of two things can happen:
    * exactly one data frame comes to **fdeIn**;
    * the data processing is cancelled.
* No data frames come to **fdeIn** if nothing is sent to **fdeOut**.

### See also
* The =[CxxFde](/doc#box/CxxFde) box implements logics for an FDE system. Concrete systems can be provided by using =[snippets](/doc#page/general-snippets).
* Example simulations
    * =[Mandelbrot set](/editor?sim=mandelbrot) (simple example);
    * =[Chaos induced by logistic mapping](/editor?sim=logistic-mapping-chaos) (another simple example, shows how to use the **nskip** parameter);
    * =[Mandelbrot set (enhanced, color-1)](/editor?sim=mandelbrot-enhanced-color-1) (shows how to use the **stop** port).
* The =[Rk4](/doc#box/Rk4) box is very similar to the FDE iterator but is designed to numerically solve ordinary differential equations (ODE).
