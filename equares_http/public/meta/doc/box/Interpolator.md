Interpolator preforms the linear interpolation of incoming data frames. Each $k$-th input frame is interpreted sa a point, ${\bf x}_k^{in}$.
Each interval $[{\bf x}_k^{in}, {\bf x}_{k+1}^{in}]$ is split into $N$ equal parts ($N$ is the value of the **count** parameter), and $N-1$ points
between these parts are written to the output port along with the incoming points. The intermediate points ${\bf x}_{k,i}$ are computed using the formula

$$
{\bf x}_{k,i} = (1-t_i){\bf x}_k^{in} + t_i{\bf x}_{k+1}^{in}, \quad t_i = \frac{i}{N}, \quad i=1,\ldots,N-1.
$$

When the first input point, ${\bf x}_1^{in}$, comes to port **input**, it is sent to port **output** immediately.
When each next input point ${\bf x}_{k+1}^{in}$ comes to port **input**, the intermediate points
${\bf x}_{k,1}, \ldots, {\bf x}_{k,N-1}$
are sent to port **output**, then ${\bf x}_{k+1}^{in}$ is sent.
