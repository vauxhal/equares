# Interactive input

## Input boxes

A running simulation can receive interactive input data from the user, and that data will affect further simulation process.
The ability to receive user input is built into boxes of several types. These _input box types_ are as follows:
* =[PointInput](/doc#box/PointInput) &mdash; for input of 2D points by clicking on an image
* =[RangeInput](/doc#box/RangeInput) &mdash; for input of points by moving sliders
* =[RectInput](/doc#box/RectInput) &mdash; for panning/zooming of an image
* =[SignalInput](/doc#box/SignalInput) &mdash; for signal input by clicking a button
* =[SimpleInput](/doc#box/SimpleInput) &mdash; for input of points by entering values

All input boxes have output port **output**. What and when they send to that port depends on box type.

## The Pause box

When a box of type =[Pause](/doc#box/Pause) is present in the simulation, it keeps simulation running after all data processing finishes.
The box listens to user input and activates corresponding input boxes as soon as user input takes place.

## Data procesing

All user input is non-blocking, which means that input boxes never wait for user input. On the other hand, an input box can only check
if there is an input event only when it is _activated_. Input boxes are activated by sending any data frame to the **activator** port
(notice, however, that =[RectInput](/doc#box/RectInput) does not have that port unless its **withActivator** parameter is set to ```true```).
Another way to activate an input box is to use the =[Pause](/doc#box/Pause) box &mdash; in that case, the input box is activated
when there is no data processing, and user input corresponding to that input box takes place.

Once an input box receives user input data, it takes an action that depends on box type. For example, it can send a data frame to the port **output**.
Or it can restart simulation.

## Vector data input

Among input box types, three of them ([PointInput](/doc#box/PointInput), =[RangeInput](/doc#box/RangeInput), and =[SimpleInput](/doc#box/SimpleInput))
allow user to interactively input _vector data_.
They all have the same logics, and only differ in how user inputs the data.

All vector data input boxes remember the data user entered at the last input event (or, at the beginning, they know that no input events have taken place).

All vector data input boxes have input port **input**. The format of this port can be any one-dimensional array. The formats of ports **input**
and **output** are the same. Vector input data boxes also have the **activator** port.

The data processing is as follows.

When a data frame comes to port **input**, a data frame is sent to port **output**. The output data is the same as the input data if no user input has
taken place on this box yet. If, however, there was user input, the box changes part of the input data frame before sending it to **output**. It replaces
some elements of input data frame with values user entered last time. Which elements are replaced depends on box parameters &mdash; refer to box documentation.

When a vector data input box is activated (either by sending a data frame to port **activator** or due to the activity of the =[Pause](/doc#box/Pause) box),
it first checks if any data is available at port **input**. If no data has been received on that port, nothing happens. User input data, if any, will be waiting
for further processing, till the box is activated next time.

If some data is available at port **input**, the box checks for user input. If there is no unprocessed user input, nothing happens.
If user input has taken place, the box reads the user input data and replaces part of last data frame obtained from **input** with new user input data.
The resulting array will be available at port **data**, but the exact behavior of the box now depends on two parameters, **restartOnInput**
and **activateBeforeRestart**.
* If **restartOnInput** is ```false```, the simulation data processing loop is exited and entered again,
  starting from the input box. The input box then sends the prepared output data frame to port **output**, and simulation continues.
* If **restartOnInput** is true, then
    * If **activateBeforeRestart** is ```true```, the prepared output data frame is sent to port **output**; otherwise, it is not sent.
    * Then simulation data processing loop is exited and entered again, starting from =[data sources](/doc#page/general-items),
      as it happens when simulation is started.

Notice that the combination **restartOnInput**=```true``` and **activateBeforeRestart**=```true``` implies that there will be no extensive data processing when the data frame
is sent to port **output** before restarting (otherwise, there probably will be no restart at all). This combination can be used, for example, if you want to specify
=[ODE solver](/doc#box/Rk4) parameters &mdash; see these simulations:

* =[Simple pendulum, phase volume evolution](/editor?sim=phase-volume-evolution) simulation, box **itime**;
* =[Forced Duffing equation, Poincare map](/editor?sim=forced-duffing-psec), box **paramInput**.

More likely you will be using the combination **restartOnInput**=```false``` and **activateBeforeRestart**=```false``` &mdash; e.g., see these simulations:

* =[Interactive phase portrait, #2](/editor?sim=interactive-phase-portrait-2) box **isInput**;
* =[Double pendulum, Poincare map (interactive 2)](/editor?sim=double-pendulum-psec-interactive2), box **isInput**.

No simulations currently use the combination **restartOnInput**=```true``` and **activateBeforeRestart**=```false```, but it could happen to be what user needs.

**_Important notice._** Even if **restartOnInput** is set to ```false```, user input events cause simulation data processing loop to be exited and entered again, which
can be treated as a simulation restart by some boxes. For example, if the **clearOnRestart** parameter of =[Canvas](/doc#box/Canvas) box is set to true,
the canvas will be cleared at each user input event.
