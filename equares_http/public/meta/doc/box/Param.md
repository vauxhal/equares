### Difference between **Param** and **Const**

The box is similar to =[Const](/doc#box/Const). Once the data processing is started, they behave identically.

The difference between **Param** and **Const** is in the source of data frame format.
- The **Param** box takes its data frame format from connection on its **output** port.
  User can only enter values that are determined by some other box connected to the **output** port.
- The **Const** box provides its own data format for the **output** port and allows
  user to add any number of elements that do not depend on what's connected to the **output** port.

Boxes of type **Param** are widely used in any simulation, since there are always boxes that require certain parameters
on their input ports.
