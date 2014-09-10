# Interactive phase portrait

The objective is to have a simulation that adds phase trajectories (of a 2nd order ODE system) that pass through points user clicks on the phase plane.

## Initial workflow
At first, let us create simulation that implements general simulation workflow.
By following instructions in this step you should obtain ?[this simulation](/editor?sim=interactive-phase-portrait-1).

### Creating and connecting simulation boxes

We need the following =[boxes](/doc#page/general-items).
- Box **ode** that implements ODE right hand side evaluation. We choose =[CxxOde](/doc#box/CxxOde) because it allows to replace the ODE system easily using =[snippets](/doc#page/general-snippets).
- Box **solver** that solves the initial value problem for an ODE system &mdash; currently we only have =[Rk4](/doc#box/Rk4) for this.
- Box **proj** of type =[Projection](/doc#box/Projection) for projecting points of the phase curve onto the phase plane (because for 2nd order ODE system phase curve points will contain three components, two state variables and the time).
- Box **canvas** of type =[Canvas](/doc#box/Canvas) that will store pixels representing the points on phase trajectories.
- Box **bitmap** of type =[Bitmap](/doc#box/Bitmap) that will store canvas data in an image file, so that file can be viewed by user.
- Three instances of boxes of type =[Param](/doc#box/Param) for providing
    - **odeParam** &mdash; parameters of the ODE system;
    - **initState** &mdash; initial values of state variables;
    - **solverParam** &mdash; solver parameters;

These boxes are created by dragging them onto the scene from the =[box list](/doc#page/editorpane-box).
It's also a good idea to name each box appropriately &mdash; set the **name** property of each box using the =[object parameter editor](/doc#page/editorpane-prop) accordingly to box names in the list above to do so.
See also [Creating boxes](/doc#page/editor-usage-createbox).

Once boxes are created, we need to =[make connections](/doc#page/editor-usage-connect) between their =[ports](/doc#page/general-items):
- ode:state &mdash; solver:rhsState
- ode:oderhs &mdash; solver:rhs
- ode:parameters &mdash; odeParam:output
- solver:parameters &mdash; solverParam:output
- solver:initState &mdash; initState:output
- solver:nextState &mdash; proj:input
- proj:output &mdash; canvas:input
- solver:finish &mdash; canvas:flush

Notice that in order to achieve better visual look of simulation scheme you can do the following:
- move boxes &mdash; just drag them (press and hold left mouse button on box, then move mouse and release button when finished);
- move ports along box perimeter &mdash; hold ```Ctrl``` key and drag box port.

See also =[Simulation scheme](/doc#page/editorpane-scheme).

### Specifying box parameters

The next step is to edit box parameters.

Let's specify the ODE first. To do so, click on the **ode** box and click the button next to the **src** parameter in the =[editor pane](/doc#page/editorpane-prop). Then locate the snippet button at the top of =[text editor](/doc#page/editorpane-text) and click it. The snippet dialog will pop up. Choose the =[Damped pendulum](/doc?name=damped-pendulum#snippet/ode) snippet in the list and then click the **Pick** button. This will copy snippet code to the text editor. Finally, click the **Ok** button in the top of text editor to save the **src** parameter.
See =[Using snippets](/doc#page/editor-usage-snippets) for more information.

Once the ODE system is specified, other parameters related to it can be set. Now you can provide damped pendulum parameters $l=1$, $g=9.8$, $n=0.1$, and initial state $q=3$, $\dot q=0$, $t=0$.

Solver parameters $h$, $n$, and $n_{out}$ should also be set in the **solverParam** box.
We do it as follows:
- set $n$ to 100000;
- set $n_{out}$ to 1;
- set $h$ (the integration step) to **h** &mdash; we will define the value of **h** in a script variable because it will be used in several places in next steps of the tutorial.

To set the value of integration step **h**, do this.
- Click on an empty area in the simulation scheme. This will bring =[simulation properties](/doc#page/editor-usage-simprop) to the =[editor pane](/doc#page/editorpane-prop).
- Click the button next to the **script** simulation property. This will open script text in the =[text editor](/doc#page/editorpane-text).
- Enter ```var h = 0.001``` in the text editor and press the **Ok** button. This will save script text.

We also need to set some more parameters.
- For **proj**, set indices to ```0,1```
- For **canvas**, set
    - **x.vmin** = -6
    - **x.vmax** = 6
    - **x.resolution** = 350
    - **y.vmin** = -12
    - **y.vmax** = 12
    - **y.resolution** = 350
- For **bitmap**, set **fileName** to ```result.png```

### Saving simulation

Choose a name for your simulation and choose the **Editor / Save** command in the menu. See =[Saving simulations](/doc#page/simfile-save) for more information.

Notice that you have to be logged in to save your simulations.

### Running simulation

Choose the **Editor / Run** command in the menu. You will see the running simulation dialog and one picture with one phase trajectory in it.

Notice that you have to be logged in to run your simulations.

## Adding point input
TODO

## Adding point output
TODO

## Adding pan/zoom
TODO

## Adding phase curve in the time decrement direction
TODO

## Marking phase curve with different colors
TODO

