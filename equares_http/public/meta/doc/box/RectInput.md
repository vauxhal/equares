### Overview

This box provides =[interactive user input](/doc#page/general-interactive-input) in the form of panning and zooming an image.

* Panning is done by pressing left mouse button on the image, moving the mouse while the button is held pressed, and then releasing the mouse button.
* Zooming is done by rotating mouse wheel. Optionally, one of keyboard keys can be held down while rotating the wheel:
    * If the ```Shift``` key is down, the rectangle is reset;
    * If the ```Ctrl``` key is down, the zooming is only done in the horizontal direction (has no effect if the **keepAspectRatio** parameter is ```true```);
    * If the ```Alt``` key is down, the zooming is only done in the vertical direction (has no effect if the **keepAspectRatio** parameter is ```true```).

The image being panned/zoomed by user is determined by its name specified in the **refBitmap** parameter.
The same name should be specified for the **fileName** parameter of some =[Bitmap](/doc#box/Bitmap) box in the simulation.

Consider the mapping ${\bf r}={\bf f}({\bf r}^{img})$ of image pixel points ${\bf r}^{img}=[x^{img},\ y^{img}]^T$ to points ${\bf r}=[x,\ y]^T$ in
rectangle ${\bf R}=\{{\bf r}\}:\quad x_{\min}\lt x\lt x_{\max},\ y_{\min}\lt y\lt y_{\max}$:
$$\begin{array}{rclcr}
x &=& x_{\min} + &\frac{x^{img}}{N_x}     & \left(x_{\max} - x_{\min}\right),\\
y &=& y_{\min} + &\frac{N_y-y^{img}}{N_y} & \left(y_{\max} - y_{\min}\right),
\end{array}
$$
where $N_x$ and $N_y$ are numbers of pixels in the image in the horizontal and vertical directions respectively.
Notice that the point $x^{img}=y^{img}=0$ is at the top-left corner of the image, while the point $x=x_{\min},\ y=y_{\min}$ is at the bottom-left corner.

The box itself does not really know about this transformation, but it is considered by client-side JavaScript code that implements panning and zooming.

The task of the box is to generate output data frames containing values $x_{\min}, x_{\max}, y_{\min}, y_{\max}$ and send them to port **output** as soon
as user pans/zooms target image. Each time an output frame is generated, the rectangle ${\bf R}$ changes, as well as the mapping ${\bf f}({\bf r}^{img})$.
Further we denote the new rectangle after panning or zooming by
${\bf R}^{new}=\{{\bf r}\}:\quad x^{new}_{\min}\lt x\lt x^{new}_{\max},\ y^{new}_{\min}\lt y\lt y^{new}_{\max}$,
and the new mapping by ${\bf f}^{new}({\bf r}^{img})$.

When panning occurs, the image point where it starts (user presses the left mouse button) is ${\bf r}^{img}_1$, and
the image point where it ends (user releases the button) is ${\bf r}^{img}_2$. The rectangle boundaries $x_{\min}, x_{\max}, y_{\min}, y_{\max}$
are changed such that point ${\bf f}({\bf r}^{img}_1) = {\bf f}^{new}({\bf r}^{img}_2)$, and rectangle sizes remain the same:
$x^{new}_{\max}-x^{new}_{\min}=x_{\max}-x_{\min}, \quad y^{new}_{\max}-y^{new}_{\min}=y_{\max}-y_{\min}$.
In other words, the point ${\bf r}$ that was seen where user pressed the mouse button, after panning will be seen where user released the button,
and there is no change in image scaling. Thus, user "drags underlying plane of points ${\bf r}$ by holding one of these points".

When zooming occurs, the image point at the mouse cursor is ${\bf r}^{img}$. The rectangle boundaries $x_{\min}, x_{\max}, y_{\min}, y_{\max}$
are changed such that point at mouse cursor does not move: ${\bf f}({\bf r}^{img}) = {\bf f}^{new}({\bf r}^{img})$, and rectangle sizes are scaled:
$$\begin{array}{l}
x^{new}_{\max}-x^{new}_{\min} = s_x\left(x_{\max}-x_{\min}\right), \quad s_x = \left[\begin{array}{rl}
    1, & \mbox{if the ```Ctrl``` keyboard key is pressed and the **keepAspectRatio** parameter is ```false```},\\
    s & \mbox{otherwise}
\end{array}\right.\\
y^{new}_{\max}-y^{new}_{\min} = s_y\left(y_{\max}-y_{\min}\right), \quad s_y = \left[\begin{array}{rl}
    1, & \mbox{if the ```Alt``` keyboard key is pressed and the **keepAspectRatio** parameter is ```false```},\\
    s & \mbox{otherwise}
\end{array}\right.\\
s = e^{-n \log c}\vphantom{=}
\end{array}
$$
In the above formula, $n$ is an integer depending on the mouse wheel rotation angle ($\pm1$ for minimum angle), and $c=1.2$.

Pan/zoom transformations are accumulated if they are done frequently, and are sent to the server after one second of user inactivity with respect to panning/zooming.

The following parameters have not been discussed above.
* **initRect** &mdash; structure containing fields that determine the initial rectangle ${\bf R}$:
    * **xmin** &mdash; initial value of $x_{\min}$;
    * **xmax** &mdash; initial value of $x_{\max}$;
    * **ymin** &mdash; initial value of $y_{\min}$;
    * **ymax** &mdash; initial value of $y_{\max}$.
* **withActivator** &mdash; if set to ```true```, the box has the **activator** port. Sending any data frame to that port
  will cause the box to check if it has new user input data and generate new output frame if necessary.
  See the **Data procesing** section in the =[Interactive input](/doc#page/general-interactive-input) page.
* **restartOnInput** similar to parameter of vector data input boxes with the same name;
  see the **Vector data input** section in the =[Interactive input](/doc#page/general-interactive-input) page.

### See also
* The **output** port can be connected to the **range** port of the following boxes
    * =[Canvas](/doc#box/Canvas)
    * =[GridGenerator](/doc#box/GridGenerator)
    * =[PointInput](/doc#box/PointInput)
* The **RectInput** box is used in many simulations. For example, see
    * =[Mandelbrot set](/editor?sim=mandelbrot)
    * =[Mandelbrot set limit points](/editor?sim=mandelbrot-lim-points)
    * =[Mandelbrot and Julia sets](/editor?sim=mandelbrot-and-julia)
    * =[Lorenz attractor](/editor?sim=lorenz-attractor)
    * =[Interactive phase portrait, #4](/editor?sim=interactive-phase-portrait-4)
