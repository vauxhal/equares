### Overview
The box is used to support interactive input.
It has no output ports and one input port, **activator**.

The box is activated when one of the following conditions is met:
* the data processing in the simulation has finished;
* any data frame has been received in port **activator**.

Typically, only the first scenario is actually realized in simulations. Therefore, the box does not need to be connected to anything else.

When the data processing in the simulation finishes, the simulation itself also finishes, but only in case there is no **Pause** box in it.
If you want to continue to wait for user input even when the data processing finishes, you really need this box.
It is not possible to wait for user input directly using a specific input box, since all interactive input is non-blocking.

When activated, the **Pause** box starts listening to interactive user input in an infinite loop. If user input takes place, the loop is exited,
and simulation continues (the continuation point depends on the logic of box that acquired user input action).
Notice that there can be many user input boxes in simulation, and user input can happen in any of them.

### See also
* step 2 of the =[Interactive phase portrait](/doc#tut/-/interactive-phase-portrait) tutorial;
* the =[Interactive phase portrait, #6](/editor?sim=interactive-phase-portrait-6) simulation;
* the following input boxes:
    * =[PointInput](/doc#box/PointInput);
    * =[RangeInput](/doc#box/RangeInput);
    * =[RectInput](/doc#box/RectInput);
    * =[SignalInput](/doc#box/SignalInput);
    * =[SimpleInput](/doc#box/SignalInput).
