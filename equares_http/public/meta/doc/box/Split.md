### Data processing

The box has a variable number (1&ndash;10) of output ports &mdash; it is determined by the value of the **outputPortCount** parameter.
The output ports are named **out_1**, **out_2**, and so on.
The data at all output ports have the same format, and this is also the format of the **input** port. The format is arbitrary.

When a data frame comes to port **input**, it is sent to all output ports **out_1**, **out_2**, and so on.

The reason to use this box is that it ensures that the data is sent to output ports in their natural order,
first to port **out_1**, then to port **out_2**, and so on.


### See also

See the following simulations using this box:
* Step 3 of the =[Interactive phase portrait](/doc#tut/-/interactive-phase-portrait) tutorial explains the case when the **Split** box is necessary.
  The =[Interactive phase portrait, #3](/editor?sim=interactive-phase-portrait-3) simulation is the result of this step of the tutorial.
* The following simulations show other examples of usage of the **Split&** box.
    * =[Phase volume evolution](/editor?sim=phase-volume-evolution)
    * =[Mandelbrot set (enhanced, color-1)](/editor?sim=mandelbrot-enhanced-color-1)
    * =[Mandelbrot set limit points](/editor?sim=mandelbrot-lim-points)
    * =[Mandelbrot and Julia sets](/editor?sim=mandelbrot-and-julia)
