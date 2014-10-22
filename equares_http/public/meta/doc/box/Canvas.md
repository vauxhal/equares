### Canvas data
Canvas provides a 2D surface for pixel data storage. The internal data of the canvas is the matrix of pixels. Each pixel contains a numeric value.
Initially, all pixels contain zero values. But during the simulation, as the canvas processes its input, some pixels may change their values.

### Data processing
Input data frames coming from the **input** port
are considered to be Cartesian coordinates $[x,y]^T$ of 2D points, if the **withInputValue** parameter is ```false```. Otherwise, if **withInputValue** is ```true```,
input data frames should also contain values, $v$, so that they are $[x,y,v]^T$. In the former case, the canvas assumes $v=1$.

These points are mapped onto the pixels of canvas. If a point falls into a canvas pixel, that pixel
obtains the value of $v$.

Canvas writes output on its **output** port only if one of the following events occurs.
* There is an input on the **flush** input port.
* The **refreshInterval** parameter has a positive value and the time elapsed since last output exceeds it.

Each output frame of the canvas is its matrix of pixel data in the current state.

### Canvas geometry
The geometry of canvas is determined by its _range_ and _resolution_ in each of its two dimensions.

The **param** parameter is a structure containing *x* and *y* fields. Each of these fields in turn are structures containing members **vmin**, **vmax**, and **resolution**.
**vmin** and **vmax** determine the range, and **resolution** determines the number of pixels. See picture below.

![canas-geom](/meta/doc/box/canvas-geom.png "Canvas geometry")

### Other canvas parameters
The **timeCheckCount** parameter controls how many input points canvas receives on port **input** between attempts to check time elapsed since last frame had been written to **output**
due to the excess of **refreshInterval** period. If you feel there might be a performance bottleneck due to the canvas, try increasing this parameter; if you see that the actual refresh rate
is much lower than 1/**refreshInterval**, but input points come in frequently enough, try decreasing this parameter.

The **clearOnRestart** parameter tells canvas to clear itself when simulation is restarted. In fact, this happens each time =[interactive user input](/doc#page/general-interactive-input) takes place.
