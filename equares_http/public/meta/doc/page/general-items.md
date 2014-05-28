# Boxes, ports, and links

Simulation is represented by a scheme consisting of a set of **boxes** and **links**. Each box has one or more **ports**. Ports can be connected by links.
An example of simulation scheme is presented in the figure below.

![](/meta/doc/page/general-scheme-example.png 'Simulatoin scheme')

Each simulation is a _data processing system_. Boxes are _data processors_, and links are _data conductors_.

## Boxes and ports
A box, as a data processor, obtains input data and produces output data. There can be different kinds of data, e.g., system state, parameters, or something else. To distinguish
between different kinds of data, the concept of **port** is employed. Each box has one or more ports. There are **input ports**, at which box reads incoming data, and
**output ports**, where box writes its output data. On the scheme, output ports are blue, and input ports are red.

![](/meta/doc/page/general-box-ports.png 'A typical box with input and output ports')

Notice that a box can have no input ports. In this case, the box is a **data source**. Or it can have no output ports, in which case it represents a **data storage**. Figure below
shows an example of data source and data storage.

![](/meta/doc/page/general-box-special.png 'Data source and data storage boxes')

## Links
