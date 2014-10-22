### Overview

This box provides =[interactive user input](/doc#page/general-interactive-input) in the form of clicking a point on an output image with the left mouse button.
It is one of _vector data input boxes_.

The image on which user clicks points is determined by its name specified in the **refBitmap** parameter.
The same name should be specified for the **fileName** parameter of some =[Bitmap](/doc#box/Bitmap) box in the simulation.

The box obtains integer coordinates of points clicked and transforms them accordingly to the current transformation.
The current transformation can be specified by either providing values for the **transform** parameter or passing data
frames to the **range** input port (e.g., by connecting it to the **output** port of the =[RectInput](/doc#box/RectInput) box).

TODO

### Data processing

TODO

### See also

The following simulations show examples of usage for the **PointInput** box:
* =[Interactive phase portrait, #2](/editor?sim=interactive-phase-portrait-2);
* =[Mandelbrot and Julia sets](/editor?sim=mandelbrot-and-julia) (the **icj** box);
* =[Double pendulum, Poincare map (interactive)](/editor?sim=double-pendulum-psec-interactive).
