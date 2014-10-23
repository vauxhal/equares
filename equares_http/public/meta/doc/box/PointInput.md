### Overview

This box provides =[interactive user input](/doc#page/general-interactive-input) in the form of clicking a point on an output image with the left mouse button.
It is one of _vector data input boxes_.

The image on which user clicks points is determined by its name specified in the **refBitmap** parameter.
The same name should be specified for the **fileName** parameter of some =[Bitmap](/doc#box/Bitmap) box in the simulation.

The box obtains integer coordinates of points clicked and transforms them accordingly to the current transformation.
The current transformation can be specified by either providing values for the **transform** parameter or passing data
frames to the **range** input port (e.g., by connecting it to the **output** port of the =[RectInput](/doc#box/RectInput) box).

The **transform** parameter is a structure with two fields, **x** and **y**, which are in turn both structures of the same type,
containing the following fields:
* **index** &mdash; the index of component of data frame (received at port **input**) to be replaced with the coordinate of the point clicked (to prepare data frame for port **output**);
* **vmin** &mdash; value that the minimum integer coordinate on the image maps to;
* **vmax** &mdash; value that the maximum integer coordinate on the image maps to;
* **resolution** &mdash; number of image pixels in the direction of the coordinate.

The coordinates of point clicked by user are computed as follows:
$$\begin{array}{rclcr}
x &=& x_{\min} + &\frac{x^{img}}{N_x}     & \left(x_{\max} - x_{\min}\right),\\
y &=& y_{\min} + &\frac{N_y-y^{img}}{N_y} & \left(y_{\max} - y_{\min}\right),
\end{array}
$$
where $x^{img}$ and $y^{img}$ are integer image coordinates (notice that the point $x^{img}=y^{img}=0$ is at the top-left corner of the image), $N_x$ and $N_y$ are numbers of pixels
in the image in the horizontal and vertical directions respectively; they are values of parameters **transform.x.resolution** and **transform.y.resolution** respectively.

Parameters $x_{\min}$, $x_{\max}$, $y_{\min}$, $y_{\max}$ determine the rectangle that the entire image maps onto. Till there are no data frames available at port **range**,
they are values of **transform.x.vmin**, **transform.x.vmax**, **transform.y.vmin**, **transform.y.vmax** respectively. If, however, a data frame comes to port **range**,
its four elements replace the values of these parameters.

Input data frames coming to port **input** are expected to be one-dimensional arrays of $n$ elements, and output data frames sent to port **output** have the same format.
Output data frames ${\bf x}^{out}=[x^{out}_0,\ldots,x^{out}_{n-1}]^T$ are obtained from input data frames ${\bf x}^{in}=[x^{in}_0,\ldots,x^{in}_{n-1}]^T$ as follows.
If there is no user input (no points have been clicked yet), then ${\bf x}^{out}={\bf x}^{in}$. Otherwise, $i_x$-th and $i_y$-th components of input
are replaced with the coordinates $x$, $y$ of the last point clicked:
$$
x^{out}_k=\left[\begin{array}{rl}
x,&\mbox{if }k=i_x,\\
y,&\mbox{if }k=i_y,\\
x^{in}_k,&\mbox{if }k\ne i_x\mbox{ and }k\ne i_y.\vphantom{=}
\end{array}\right.
$$
The indices $i_x$ and $i_y$ are the values of parameters **transform.x.index** and **transform.y.index** respectively.

The data processing for this box, as well as parameters **restartOnInput** and **activateBeforeRestart**, are described in the
**Data procesing** and **Vector data input** sections of the =[Interactive input](/doc#page/general-interactive-input) page.

### See also

The following simulations show examples of usage for the **PointInput** box:
* =[Interactive phase portrait, #2](/editor?sim=interactive-phase-portrait-2);
* =[Mandelbrot and Julia sets](/editor?sim=mandelbrot-and-julia) (the **icj** box);
* =[Double pendulum, Poincare map (interactive)](/editor?sim=double-pendulum-psec-interactive).
