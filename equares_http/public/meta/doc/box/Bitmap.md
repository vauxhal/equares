### Data processing
The box writes each =[data frame](/doc#page/general-data) coming to its **input** port to the output file. The name of the file is determined by the **fileName** parameter.

Input frame is expected to contain 2D array of values. Array sizes determine the resolution of the output image. Each array element is represented by a pixel of the
output image. The color of the pixel is determined by the _color map_.

Typically a bitmap box is connected to a =[canvas box](/doc#box/Canvas).

### Specifying color map
The color map can be specified using the **valueRange** and **colorMap** parameters as follows.

Each value from the data frame array on the **input** port, $v$, is mapped to normalized value, $t\in[0,1]$ by using the formula
$$
  t(v) = \left\{\begin{array}{rl}
  0, & \mbox{if}\quad v < v_{\min} \\
  \frac{v - v_{\min}}{v_{\max} - v_{\min}}, & \mbox{if}\quad v\in[v_{\min}, v_{\max}] \\
  1, & \mbox{if}\quad v > v_{\max}
  \end{array}\right.
$$
In the above formula, the parameters $v_{\min}$ and $v_{\max}$ are **valueRange.vmin** and **valueRange.vmax** respectively.

Normalized values $t(v)$ are mapped to colors by using the linear interpolation of colors in the **colorMap** array.
Each element of the array is a pair of two parameters, **pos** and **color**. The **color** parameter is a hexadecimal number
having the ```rrggbb``` format (one byte for each of red, green, and blue components); it specifies the color
at position $t$=**pos**.

Before color map is used in data processing, the following algorithm is applied to have a valid color map.
1. All elements of **colorMap** are sorted in the **pos** increment order.
2. If first element has a positive value of **pos**, or if there is no first element (color map is empty), a new element
   {**pos**=0, **color**=ffffff} is inserted at the beginning.
3. If last element has **pos**<1, a new element {**pos**=1, **color**=000000} is added at the end.
4. If an element at **pos**=0 is missing, it is inserted (with interpolated color).
5. If an element at **pos**=1 is missing, it is inserted (with interpolated color).
6. Any elements with **pos**<0 and **pos**>1 are removed.

### See also
The following simulations show examples of usage of a colored image.
- =[Interactive phase portrait, #6](/editor?sim=interactive-phase-portrait-6)
- =[Mandelbrot set (enhanced, color-1)](/editor?sim=mandelbrot-enhanced-color-1)
