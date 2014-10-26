### Overview

The box transforms scalar input value coming to port **input** into scalar output value of 0 or 1,
accordingly to the result of comparison with the threshold value. The output values of 1 are always sent to port **output**.
The output values of 0 are only sent to port **output** if the **quiet** parameter is set to ```false```; otherwise, if
**quiet** is ```true```, the output of zero values is suppressed.

The box has input port **threshold** that can optionally be connected to something else. If it is connected, the threshold value $T$ is considered to be
the value of last scalar data frame received on that port. Otherwise, if the port **threshold** is not connected, $T$ is the value of the **thresholdValue** parameter.

The box has the **param** parameter; its value determines how output value $v$ is computed from input value $x$. Below is the list
of possible values of **param** and logical expressions corresponding to them. If the result of expression is ```true```,
output value $v$ is 1; otherwise, it is 0.
* ```less```: $x\lt T$
* ```less_or_equal```: $x\le T$
* ```greater```: $x\gt T$
* ```greater_or_equal```: $x\ge T$
* ```equal```: $x=T$
* ```not_equal```: $x\ne T$

### Data processing

When a data frame with scalar value $x$ comes to port **threshold**, its value is assigned to the current threshold value, $T$; no further processing is done.

When a data frame comes to port **input**, one of the following two things happens.
* If the **threshold** port is connected but no data frames are received on that port yet, the data processing is cancelled because
  the threshold value $T$ is unknown.
* Otherwise, the scalar output value $v$ is computed (see overview above). Then, if $v=1$ or **quiet** is ```false```, $v$ is sent to port **output**.

### See also
* The =[Valve](/doc#box/Valve) box can be connected by its **valve** input port to port **output** of the **ThresholdDetector** box.
* The following simulations show examples of usage of the **ThresholdDetector** box.
    * =[Mandelbrot set](/editor?sim=mandelbrot)
    * =[Mandelbrot set (enhanced, color-1)](/editor?sim=mandelbrot-enhanced-color-1)
    * =[Julia set for quadratic polynomial](/editor?sim=julia)
