### Canvas data
Canvas provides a 2D surface for pixel data storage. The internal data of the canvas is the matrix of pixels. Each pixel contains a numeric value.
Initially, all pixels contain zero values. But during the simulation, as the canvas processes its input, some pixels may change their values.

### Data processing
Input data frames coming from the **input** port
are considered as Cartesian $(x,y)$ coordinates of 2D points. These points are mapped onto the pixels of canvas. If a point falls into a canvas pixel, that pixel
obtains the value of 1.

Canvas writes output on its **output** port only if one of the following events occurs.
* There is an input on the **flush** input port.
* The **refreshInterval** parameter has a positive value and the time elapsed since last output exceeds it.

Each output frame of the canvas is its matrix of pixel data in the current state.

### Canvas geometry
The geometry of canvas is determined by its _range_ and _resolution_ in each of its two dimensions.

The **param** parameter is a structure containing *x* and *y* fields. Each of these fields in turn are structures containing members **vmin**, **vmax**, and **resolution**.
**vmin** and **vmax** determine the range, and **resolution** determines the number of pixels. See picture below.

![canas-geom](/meta/doc/box/canvas-geom.png "Canvas geometry")
