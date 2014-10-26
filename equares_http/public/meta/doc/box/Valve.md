### Overview

The box is designed to control the passing of data frames from port **input** to port **output** controlled by value at port **valve**.
In other words, it is a filter.

Ports **input** and **output** must have the same format; the format can be arbitrary.

### Data processing

The box has two boolean internal state variables, $s^i$  and $s^v$ indicating if there are unprocessed data frames at ports **input** and **output** respectively.
Initially, they are both ```false```.

When a controlling data frame comes to port **valve**, $s^v$ is set to ```true```
(note: in contrast to the =[Join](/doc#box/Join) box, there is no requirement that $s^v$ should be ```false``` at this moment).
Current controlling value $v$ is set to ```true``` if the controlling data frame element is nonzero, and to ```false``` otherwise.
Then, if $s^i$ is ```true```, further processing is done: last data frame received at port **input** is taken; it is sent
to port **output** if $v$ is ```true``` and not sent if $v$ is ```false```. Then $s^v$ and $s^i$ are both set to ```false```.

When an input data frame comes to port **input**, $s^i$ is set to ```true```
(note: in contrast to the =[Join](/doc#box/Join) box, there is no requirement that $s^i$ should be ```false``` at this moment).
Then, if $s^v$ is also ```true```, further processing is done exactly the same way as explained above: last data frame received at port **input** is taken; it is sent
to port **output** if $v$ is ```true``` and not sent if $v$ is ```false```. Then $s^v$ and $s^i$ are both set to ```false```.

### See also
The following simulations show examples of usage of the **Valve** box.
* =[Ince-Strutt diagram](/editor?sim=mathieu-stability)
* =[Mandelbrot set](/editor?sim=mandelbrot)
* =[Julia set for quadratic polynomial](/editor?sim=julia)
