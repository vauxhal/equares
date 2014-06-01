# Port data

## Data frame sequence

Port data is a sequence of **frames**.
Once a simulation is defined, each data frame at each port has a fixed **format**.
When simulation is running, each output port provides certain element of the data frame sequence, or provides no data at all if it is not ready.
As simulation run, boxes can supply new data frames on their output ports.

Each input port contains the same data frame as the output port connected to it.

## Data frame format

A data frame has a certain format. Frames always contain real numbers. A format specifies sizes of one-, two-, or multi-dimensional array.
So frame data is generally a multi-dimensional array.

_Port format is a list of sizes of a multi-dimensional array_. For example, format [150, 100] specifies that each port frame is a two-dimensional 150x100 array.

A box can provide a format for its ports, or can set the format as unspecified, or just declare array dimension (e.g., see =[Bitmap](/doc#box/Bitmap)).
If port format specification is missing or incomplete, the format is determined from connections with other ports.

Two ports can only be connected if their formats appear to be the same.

![](/meta/doc/page/general-box-links.png 'An example of valid links')

## Format compatibility

If formats of two ports are different, the link between them is shown as a red line, and an error message is issued on an attempt to run simulation:

![](/meta/doc/page/general-box-links-bad.png 'Invalid links are shown as red lines')

In this example the solver would say

<span style="color: red;">Error: Incompatible port formats at link double pendulum:oderhs -> canvas:input</span>

Another wrong situation is that a box requires an array of certain dimension, but it appears that the actual format has a different dimension:

![](/meta/doc/page/general-box-links-bad-2.png 'Link is valid, but port format is bad for bitmap')
![](/meta/doc/page/general-box-links-bad-3.png 'Error message can be viewed by hovering on status rectangle')

In this example, the bitmap box requires 2D data, but the oderhs port of the pendulum box provides 1D data. The link is valid because
the input port of the bitmap box is now forced to have the same format as the oderhs output of the pendulum box. As long as the bitmap
recognizes wrong input port format, it changes its status to red (invalid) and reports error that can be viewed by hovering the status rectangle.

## Data hints

In addition to sizes of the multi-dimensional array, a format can specify **data hints**. So far these hints are only used for one-dimensional data,
but this can be generalized to multi-dimensional data.

_Data hints are text labels attached to all indices of multi-dimensional array, for each dimension._
For 1D data, this means that text labels just name elements of the array. For example, ODE boxes such as
=[pendulum](/doc#box/Pendulum) and =[double pendulum](/doc#box/DoublePendulum) label elements of all their ports.

Data hints are useful, for example, when a box needs a set of named parameters on its ports.

## Format propagation

Some boxes do not specify any port format. Instead, they just take the format of connected port.
This behavior may be called **format propagation along links**.

One example here is the =[Param](/doc#box/Param) source box. It takes format of connected port, together with any data hints, and provide a way for user
to enter port data as a set of named parameters.

There are also boxes that provide some format on one or more of their ports as soon as formats of some other ports, and probably some parameters, are provided.
For example, the =[Projection](/doc#box/Projection) box defines its output port format as soon as the input port format is known, and projection indices are set.
A more complex example is the =[Rk4](/doc#box/Rk4) solver box that deduce formats for its initState and nextState ports basing on the formats of
its rhs and rhsState ports. Such behavior can be called **format propagation through boxes**.
