### Overview

The box produces one output data frame and sends it to port **output** for each input data frame coming to port **input**.

The format of the input port is arbitrary; the only limitation is that there must be at least one element. Input data frame is treated as a
vector ${\bf x} = [x_0,\ldots,x_{n-1}]^T$, where $n>0$ is the size of the vector.

The output data frame consists of one element, $s$, computed from ${\bf x}$ according to the value of the **param** parameter.
Below is the list of possible **param** values and how $s$ is computed for each of them.

* ```norm_1```: $s=\sum_{k=0}^{n-1} \left|x_k\right|$
* ```norm_2```: $s=\sqrt{\sum_{k=0}^{n-1} \left(x_k\right)^2}$
* ```norm_inf```: $s=\max_{k=0}^{n-1} \left|x_k\right|$
* ```min```: $s=\min_{k=0}^{n-1} x_k$
* ```max```: $s=\max_{k=0}^{n-1} x_k$

# See also
* The =[ThresholdDetector](/doc#box/ThresholdDetector) box is often combined with the **Scalarize** box.
* The following simulations show examples of usage of the **Scalarize** box.
    * =[Mandelbrot set](/editor?sim=mandelbrot)
    * =[Mandelbrot set (enhanced, color-1)](/editor?sim=mandelbrot-enhanced-color-1)
    * =[Julia set for quadratic polynomial](/editor?sim=julia)
