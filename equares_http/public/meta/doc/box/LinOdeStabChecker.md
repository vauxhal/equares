### Overview

The box is designed to analyze the stability of linear ODE systems with periodic coefficients and zero right hand side.

The coefficients are considered to be periodic functions of time, with period $T$. The box does not need to know $T$;
however, the solver connected to the box should return the state ${\bf x}(T)$ when given initial state ${\bf x}(0)$.

The box is connected to an ODE solver by input port **initState**, to pass initial state ${\bf x}(0)$ to the solver, and by input port **solution**,
to obtain the system state ${\bf x}(T)$.

Stability analysis is performed as soon as any data frame comes to input port **activator**. After that, the result is written to output port **result**.
The output value is 1 if the system is stable, and 0 if unstable.

The stability analysis is done as follows. Let $n$ be the number of state variables, not including the time $t$, so that system state vector is
$$
{\bf x}=\left[x_1,\ldots,x_n, t\right]^T =
\left[\begin{array}{c}{\bf y}\\ t\end{array}\right], \quad {\bf y}=\left[x_1,\ldots,x_n\right]^T.
$$

The ODE system whose stability we are analyzing should have the form
$$
\dot{\bf y} = {\bf A}(t){\bf y}, \quad {\bf A}(t+T)={\bf A}(t).
$$

The box takes $n$ initial states $t=0$, ${\bf y}^1(0), \ldots {\bf y}^n(0)$ such that
$$
{\bf y}^k(0) = \left[y^k_1(0),\ldots y^k_n(0)\right]^T,
\quad y^s_k(0) = \delta^s_k \equiv \left[\begin{array}{rcl} 1, &\mbox{if}& s=k\\ 0, &\mbox{if}& s\ne k \end{array}\right.
$$
In other words, these vectors make up the $n$-by-$n$ identity matrix: ${\bf Y}(0)=\left[{\bf y}^1(0),\ldots {\bf y}^n(0)\right] = I$, $I_{ks}=\delta_{ks}$.
For each initial state, ${\bf x}^k(0)=\left[\begin{array}{c}{\bf y}^k(0)\\ 0\end{array}\right]$, the initial value problem is solved and
${\bf y}^k(T)$ are obtained. Then the stability is determined by characteristic multipliers $\rho_k$ &mdash; the eigenvalues of the monodromy matrix
(system fundamental matrix computed at period $T$):
$$
\begin{array}{rcl}
{\bf M}{\bf z}_k &=& \rho_k{\bf z}_k,\\
{\bf M}&=&{\bf Y}(T)=\left[{\bf y}^1(T),\ldots {\bf y}^n(T)\right]
\end{array}
$$
The solution is stable if the absolute value of each multiplier does not exceed 1, and is unstable if there is at least one $k$ such that $|\rho_k|>1$.

Practically, in many cases, for multipliers we have $|\rho_k|=1$ (for all $k$) if the system is stable, and $|\rho_k|>1$ (for one or more $k$) if the system is unstable.
For such systems, numerical solution will most likely always give $1<|\rho_k|\lt1+\varepsilon$ if the system is stable, where $\varepsilon\ll 1$ is a small value.
Therefore, the stability detection is based on checking inequality $|\rho_k|\lt1+\varepsilon$ rather than $|\rho_k|\le1$, and $\varepsilon=10^{-5}$ is a hardcoded constant.

### Data processing
To start analyzing the stability of a linear ODE system, it is necessary to pass any data frame to port **activator**. This initiates the process of stability
detection described above. For each input data frame at port **activator**, one data frame is sent to port **result**. The output data frame sent to port **result** contains
one number: 1 if system is stable and 0 if not.

The box expects that the output of a single data frame to port **initState** results in a single data frame at port **solution**, and if it does not send anything
to port **initState**, nothing comes to port **solution**.

### Notes
Currently only second order ODE systems are supported, $n=2$. There are plans to support arbitrary system size; this would rely on an implementation of the
?[LAPACK](http://www.netlib.org/lapack/) library, and we are planning to use AMD's ?[ACML](http://developer.amd.com/tools-and-sdks/cpu-development/amd-core-math-library-acml/).

### See also
- The =[Rk4](/doc#box/Rk4) solver is the only box currently capable of solving initial value problems for ODE systems, so it is currently the only box
  that you can connect to ports **initState** and **solution**.
- The =[Ince-Strutt diagram](/editor?sim=mathieu-stability) simulation shows an example of usage for this box.
