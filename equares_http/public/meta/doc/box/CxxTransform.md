### Overview
The box allows user to define an arbitrary mapping of input data frames to output data frames; in addition, the mapping can depend on parameters.
User defines the mapping by providing its C++ code. The code also defines data frame formats of all ports.

### Data processing
When a new data frame is passed to the **input** input port, the box transforms it and sends the result to the **output** port.
However, to make this processing possible, parameters have to be available. This means that
* at least one data frame must be passed to the **parameters** port before anything can be sent to **output**;
* or there are no parameters, i.e., data frame size for port **parameters** is zero.
Therefore, the processing of **input** data frame is cancelled if no data frames have come to the **parameters** port yet, but some parameters are required.
The transformation of input data is done with the last set of parameters passed to the **parameters** port.

### Source code requirements
The code in the **src** parameter must contain an arbitrarily named class containing the following public methods:
- ```int paramDimension() const;```<br/>
  must return the dimension of data frames that must be passed to the **parameters** port.
- ```const int* paramSizes() const```<br/>
  must return a pointer to array of dimension sizes of data frames that must be passed to the **parameters** port.
  Notice that the size of the array must be equal to the value returned by ```paramDimension()```; it is recommended
  to declare the array of dimension sizes as a static local variable and return it.
- ```const char* paramNames() const;```<br/>
  must return a C-string containing comma-separated list of parameter names.
  The method is allowed to return an empty string or a null pointer.
- ```int inputDimension() const;```<br/>
  must return the dimension of data frames that must be passed to the **input** port.
- ```const int* inputSizes() const```<br/>
  must return a pointer to array of dimension sizes of data frames that must be passed to the **input** port.
  Notice that the size of the array must be equal to the value returned by ```inputDimension()```; it is recommended
  to declare the array of dimension sizes as a static local variable and return it.
- ```const char* inputNames() const;```<br/>
  must return a C-string containing comma-separated list of names of input data frame elements.
  The method is allowed to return an empty string or a null pointer.
- ```int outputDimension() const;```<br/>
  must return the dimension of data frames sent to the **output** port.
- ```const int* outputSizes() const```<br/>
  must return a pointer to array of dimension sizes of data frames sent to the **output** port.
  Notice that the size of the array must be equal to the value returned by ```outputDimension()```; it is recommended
  to declare the array of dimension sizes as a static local variable and return it.
- ```const char* outputNames() const;```<br/>
  must return a C-string containing comma-separated list of names of output data frame elements.
  The method is allowed to return an empty string or a null pointer.
- ```void prepare(const double *param);```<br/>
  called each time a new data frame comes to the **parameters** port. The method is given the pointer to the array of parameter values, ```param```
  (the number of elements in the array is the product of all dimension sizes returned by ```paramSizes()```);
  it can do whatever it wants (e.g., do nothing); a common use case for this method is
  to compute some combinations of parameters that allow to execute the ```transform()``` method (see below) faster. The combinations
  of parameters computed by this method must be stored in class member variables.
- ```void transform(double *out, const double *param, const double *input) const;```<br/>
  called each time a new data frame comes to the **input** port, but only if some data has already been sent to **parameters**,
  or if there are no parameters at all (the latter case is implemented by returning 1 from ```paramDimension()``` and [0] from ```paramSizes()```).
  The method must compute the transformation of input to output. The result will be sent to the **output** port.
  The method is given the following arguments:
    - ```out``` &mdash; pointer to array where values of transformation result should be stored. These values will be
      sent to the **output** port as soon as this method finishes.
      Notice that the number of elements in the array is the product of all dimension sizes returned by ```outputSizes()```.
    - ```param``` &mdash; pointer to array of parameters last time passed to the **parameters** port.
      Notice that the number of elements in the array is the product of all dimension sizes returned by ```paramSizes()```.
    - ```input``` &mdash; pointer to array of input variables passed to the **input** port.
      Notice that the number of elements in the array is the product of all dimension sizes returned by ```inputSizes()```.

Optionally, you can provide a description of your system that will be displayed in the =[info pane](/doc#page/editorpane-info). The description block
is necessary if you are going to make you code available as a =[snippet](/doc#page/general-snippets) &mdash; see =[Using snippets](/doc#page/editor-usage-snippets) for more information.

### Providing FDE source code
A working code example can be taken from the **srcExample** parameter and then edited as necessary.

Another way to provide source code is to use =[snippets](/doc#page/general-snippets). The list of transformation snippets currently available can be found in
the [Transformation snippets](/doc#snippet/transform) page.
