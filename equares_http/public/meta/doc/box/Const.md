### Difference between **Const** and **Param**

The box is similar to =[Param](/doc#box/Param). Once the data processing is started, they behave identically.

The difference between **Const** and **Param** is in the source of data frame format.
- The **Const** box provides its own data format for the **output** port and allows
  user to add any number of elements that do not depend on what's connected to the **output** port.
- The **Param** box takes its data frame format from connection on its **output** port.
  User can only enter values that are determined by some other box connected to the **output** port.

### See also
- The [Simple pendulum, phase volume evolution](/editor?sim=phase-volume-evolution) example shows an example
  of usage of the **Const** box (see boxes **cf** and **ci**).
