### Overview
The box allows user to define an FDE (Finite Difference Equation) system by providing its C++ code.
Its job is to map state variables coming to the **state** input port onto the state variables of the next system state,
and to write them to the **nextState** output port. Data frames coming to the **parameters** input port provide
values of additional parameters for the mapping.

### Data processing
When a new data frame containing the FDE system state is passed to the **state** input port, the box generates next system state and sends it to the **nextState** port.
However, to make this processing possible, parameters have to be available. This means that at least one data frame must be passed to the **parameters** port before anything can
be sent to **nextState**. Therefore, the processing of **state** data frame is cancelled if no data frames have come to the **parameters** port yet.
The evaluation of the next system state is done with the last set of parameters passed to the **parameters** port.

The **state** and **nextState** ports are typically connected to the **fdeOut** and **fdeIn** ports of an =[FDE iterator](/doc#box/FdeIterator) box respectively,
and the **parameters** port can be connected, for example, to the output port of a [Param](/doc#box/Param) box.

### Source code requirements
The code in the **src** parameter must contain an arbitrarily named class containing the following public methods:
- ```int paramCount() const;```<br/>
  must return the number of parameters that must be passed to the **parameters** port
- ```const char *paramNames() const;```<br/>
  must return a C-string containing comma-separated list of parameter names.
  The method is allowed to return an empty string or a null pointer, but that is not recommended if the number of parameters is positive.
- ```int varCount() const;```<br/>
  must return the number of state variables (this will determine the format for ports **state** and **nextState**).
- ```const char *varNames() const;```<br/>
  must return a C-string containing comma-separated list of state variable names.
  The method is allowed to return an empty string or a null pointer, but that is not recommended.
- ```void prepare(const double *param);```<br/>
  called each time a new data frame comes to the **parameters** port. The method is given the pointer to the array of parameter values, ```param```
  (the array contains as many elements as ```paramCount()``` returns);
  it can do whatever it wants (e.g., do nothing); a common use case for this method is
  to compute some combinations of parameters that allow to execute the ```nextState()``` method (see below) faster. The combinations
  of parameters computed by this method must be stored in class member variables.
- ```void nextState(double *out, const double *param, const double *state) const;```<br/>
  called each time a new data frame comes to the **state** port, but only if some data has already been sent to **parameters**.
  The method must compute next state of the system. That state will be sent to the **nextState** port.
  The method is given the following arguments:
    - ```out``` &mdash; pointer to array where values of state variables at the next state should be stored. These values will be
      sent to the **nextState** port as soon as this method finishes.
      Notice that the array contains as many elements as ```varCount()``` returns.
    - ```param``` &mdash; pointer to array of parameters last time passed to the **parameters** port.
      Notice that the array contains as many elements as ```paramCount()``` returns.
    - ```state``` &mdash; pointer to array of current state variables passed to the **state** port.
      Notice that the array contains as many elements as ```varCount()``` returns.

Optionally, you can provide a description of your system that will be displayed in the =[info pane](/doc#page/editorpane-info). The description block
is necessary if you are going to make you code available as a =[snippet](/doc#page/general-snippets) &mdash; see =[Using snippets](/doc#page/editor-usage-snippets) for more information.

### Providing FDE source code
A working code example can be taken from the **srcExample** parameter and then edited as necessary.

Another way to provide source code is to use =[snippets](/doc#page/general-snippets). The list of FDE snippets currently available can be found in
the [FDE snippets](/doc#snippet/fde) page.

### Difference between CxxFde and CxxOde

In fact, the only logical difference between the =[CxxOde](/doc#box/CxxOde) and **CxxFde** boxes is that **CxxOde** includes the time in its set of state variables,
while **CxxFde** does not include the time.
