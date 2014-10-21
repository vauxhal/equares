### Overview

The box is similar to =[Param](/doc#box/Param) in that it takes data format from connection on its **output** port,
so user can only enter values that are determined by some other box connected to the **output** port.

The **data** parameter holds the sequence of elements, each of which is turned into a data frame and sent to port **output**.

The box has two operation modes.
* When the **withActivator** parameter is ```false```, the box is a data source, since there are no input ports.
  Once activated by the system, it sends all array elements of the **data** parameter to port **output**.
* When the **withActivator** parameter is ```true```, the box is a data processor. It now has one input port,
  **activator**. All array elements of the **data** parameter are sent to port **output** as soon as
  any data frame comes to port **activator**.

### See also
* The =[Param](/doc#box/Param) box;
* The =[Interpolator](/doc#box/Interpolator) box;
* The following simulations:
    * [Chaos induced by logistic mapping](/editor?sim=logistic-mapping-chaos) (notice that **data** has two array elements, and **Interpolator** attached to port **output**
      generates many intermediate frames);
    * [Simple pendulum, phase portrait](/editor?sim=simple-pendulum-phase-portrait) (notice the use of **Interpolator** attached to port **output**);
    * [Interactive phase portrait, #5](/editor?sim=interactive-phase-portrait-5) (example of use for the **activator** port).
