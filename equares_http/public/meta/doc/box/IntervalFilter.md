The box interprets its input data as consecutive points ${\bf x}^{in}_k$ ($k=1,2,\ldots$) of curve ${\bf x}_k(t)$, where $t$ is the time
or other parameter that must be monotonously increasing.
The value of time at $k$-th point, $t_k^{in}$
should be included into each point; it is considered to be the element of data frame at index specified by parameter **param.index**.

The output consists of points  ${\bf x}_s$ ($s=1,2,\ldots$) that correspond to time instants $t_s$ such that $t_{s+1}-t_s=T$ is a constant
equal to the value of parameter **param.interval**. Points ${\bf x}_s$ are computed as if the curve ${\bf x}_k(t)$ were piecewisely linear,
with vertices at points ${\bf x}^{in}_k$.

Time instants $t_s$ are computed as follows.

$$
t_1 = \left\lceil \frac{t^{in}_1-c}{T} \right\rceil T+c, \qquad
t_s = t_{s-1} + T, \quad s=2,3,\ldots,
$$
so that the condition $t_1\ge t_1^{in}$ always holds. The constant $c$ in the above formula is the value of parameter **param.offset**.
