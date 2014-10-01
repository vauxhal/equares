# Interactive phase portrait

The objective is to have a simulation that adds phase trajectories (of a 2nd order ODE system) that pass through points user clicks on the phase plane.

## Step 1. Initial workflow
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
    - **param.x.vmin** = -6
    - **param.x.vmax** = 6
    - **param.x.resolution** = 350
    - **param.y.vmin** = -12
    - **param.y.vmax** = 12
    - **param.y.resolution** = 350
- For **bitmap**, set **fileName** to ```result.png```

### Saving simulation

Choose a name for your simulation and choose the **Editor / Save** command in the menu. See =[Saving simulations](/doc#page/simfile-save) for more information.

Notice that you have to be logged in to save your simulations.

### Running simulation

Choose the **Editor / Run** command in the menu. You will see the running simulation dialog and one picture with one phase trajectory in it.

Notice that you have to be logged in to run your simulations.

## Step 2. Adding point input
So far our simulation is only able to produce and display just one phase curve, starting at point specified by the parameter in the **initState** box.
The next step is to add some interactivity. Now we are going to let user click points right on the bitmap and display phase trajectories starting at
these points.

By following instructions in this step you should obtain ?[this simulation](/editor?sim=interactive-phase-portrait-2).

Interactive point input is provided by the =[PointInput](/doc#box/PointInput) box. The box works as a filter that replaces part of original input
by coordinates of point clicked by the user.

Since we are going to interactively input initial sate for the phase trajectory, we should place the point input box (let's name it **isInput**, which means
*i*nitial *s*tate *Input*) between **initState** and **solver**. Now connections should be edited: remove the link solver:initState &mdash; initState:output
by clicking on it, then add two new links:
- solver:initState &mdash; isInput:output;
- isInput:input &mdash; initState:output.

Now data passes through our **isInput** box on its way from **initState** to **solver**.

The point input box needs to be set up. Basically it needs three things:
- which bitmap to click;
- which elements of data coming to the **input** port to replace with coordinates of points clicked;
- the coordinate mappings in vertical and horizontal directions.

So we should set the following parameters of the **isInput** box:
- **refBitmap** = ```result.png``` (this is the name of the output bitmap file set in the **bitmap** box, in the **fileName** parameter);
- **transform.x.index** = 0 (we will replace initial coordinate $q$, that has index 0, with the abscissa of the point clicked);
- **transform.y.index** = 1 (we will replace initial speed $\dot q$, that has index 1, with the ordinate of the point clicked);
- Coordinate range and resolution of the canvas that produces the bitmap (same as **param** of the **canvas** box):
    - **transform.x.vmin** = -6
    - **transform.x.vmax** = 6
    - **transform.x.resolution** = 350
    - **transform.y.vmin** = -12
    - **transform.y.vmax** = 12
    - **transform.y.resolution** = 350

Notice that there is another way of specifying coordinate range and resolution (common to both canvas and point input); we will go that way further,
when interactive pan/zoom is added at step 4.

We will also need a box of type =[Pause](/doc#box/Pause). Add it and name it **pause**. No connections to this box are required.
The reason for that box is that all interactive input is non-blocking; as such, our simulation will start from displaying phase trajectory started at point
specified in the **initState** box. And then, as soon as the trajectory is displayed, the simulation is like already finished, right? But we actually need
to wait for user to click a point on the bitmap and display next phase trajectory, and wait again, and so on. The **pause** box is designed to support this
pattern. Once there is nothing more to do in simulation, pause box (if any) is activated; it then waits for interactive user input (if any), and once the input
is obtained, the simulation continues because the input box generates new data frame on its output port.

## Step 3. Adding point output
At this step we modify the simulation such that all initial points are written to a text file, so the user sees their coordinates.

Following this step will result in ?[this simulation](/editor?sim=interactive-phase-portrait-3).

Basically we need to add a box of type =[Dump](/doc#box/Dump) (name it **clickedPoints**, also name its output file by setting parameter **fileName** to
```clickedPoints.txt```) and add link isInput:output &mdash; clickedPoints:input.
That should work, right? Nope... Unfortunately, this will not work fine. The problem is that the data written to a file is buffered, and no one knows
when the file will actually contain what you wrote to it. As a result, the user will never see the points he/she clicked.
Okay, but the **clickedPoints** has the **flush** port! We can send any data to that port to ask all file
buffers to be written to file. So, adding the link isInput:output &mdash; clickedPoints:flush should update the file with initial points each time a new point is
clicked. This seems to do the job, right? Well, not really the best solution. If you do so, it will work for the first time. But let us further remove
the isInput:output &mdash; clickedPoints:input link and add it again. What's happening now? Right, each time user clicks a point, he/she sees the previously
clicked point instead of the last clicked point. Why? Because the order of link activation is unspecified in this case. Although we have two identically
looking simulation schemes, they don't do the same.

To provide a more robust solution of our problem, we need a box of type =[Split](/doc#box/Split) (name it **isSplit**, meaning *i*nput *s*tate *S*plit).
Split box has one input port, named **input**, and a variable number of output ports. In our case, three output ports are necessary, so we should set the **outputPortCount**
parameter of **isSplit** to 3. The box will then have output ports **out_1**, **out_2**, and **out_3**. As you probably already guessed, the point here is that the split box
will activate its output ports in fixed order: as it obtains a data frame on port **input**, it first sends that data to **out_1**, then to **out_2**, and finally to **out_3**.

This step is completed by removing link solver:initState &mdash; isInput:output and adding the following new links:
- isInput:output &mdash; isSplit:input;
- isSplit:out_1 &mdash; solver:initState (let's display the phase trajectory first);
- isSplit:out_2 &mdash; clickedPoints:input (then let's write the initial state to the text file);
- isSplit:out_3 &mdash; clickedPoints:flush (important to flush *after* write).

## Step 4. Adding pan/zoom
TODO

## Step 5. Adding phase curve in the time decrement direction
TODO

## Step 6. Marking phase curve with different colors
TODO

