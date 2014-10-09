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
At this step we add another input box to the simulation; the box provides the way to interactively pan and zoom canvas.

Following this step will result in ?[this simulation](/editor?sim=interactive-phase-portrait-4).

We need to add a box of type =[RectInput](/doc#box/RectInput) (and we name it **pan-zoom**). The box has just one output port, named **output**.
The port generates data frames each containing four numbers: $x_{\min}$, $x_{\max}$, $y_{\min}$, and $y_{\max}$. These numbers determine the ranges
for abscissa and ordinate on the canvas. A new frame is generated each time user rotates the mouse wheel on target bitmap, or drags (the contents of)
the target bitmap.

Importantly, the **pan-zoom** box is now the only one where we have to define the initial coordinate ranges for both **canvas** and **isInput** (previously we
entered these ranges right in the two boxes). Now these ranges are transferred from **pan-zoom** to **canvas** and **isInput** if we connect the port pan-zoom:output
to input ports canvas:range and isInput:range.

Similarly to the **isInput** box, we have to specify the **refBitmap** parameter of **pan-zoom** (set it to ```result.pbg```, since this is our only output image), so
it can know where we are going to do pan/zoom.

And the last bot not least important thing to do is to set the **restartOnInput** parameter of **pan-zoom** to ```true``` (check the box next to parameter name
in the =[parameter editor](/doc#page/editorpane-prop)). This will restart simulation from the beginning after each pan/zoom user action. As a result,
the simulation will clear canvas and show phase curve starting at the last clicked point.

## Step 5. Adding phase curve in the time decrement direction
Our simulation displays parts of phase curves starting at points clicked by user. It would be nice, however, if we could see each phase curve _passing through_ the
point clicked, rather than starting at that point. At this step we accomplish this task and end up with =[this simulation](/editor?sim=interactive-phase-portrait-5).

Obviously, to make phase curve pass through some point, we can generate two parts of it. One part is just the same as before. The other one starts at the
same point, but this time we go in time decrement direction (we can always do it by supplying a negative value of the integration step to **solver**).

So we need to start numerical integration twice per initial point, each time with different sign of the intefration step. To do so, we need to do the following
modifications.
- Replace type of the **solverParam** box to =[ParamArray](/doc#box/ParamArray).
- Set the **withActivator** parameters of **solverParam** to ```true``` (check the box  next to parameter name in the =[parameter editor](/doc#page/editorpane-prop)).
  After that, the box will have the **activator** port. When any data frame comes to this port, the box outputs all elements of its data array to the **output** port.
- Increase output port of **isSplit** by one, so that now the **outputPortCount** parameter is 4. We need the additional output to activate **solverParam**
  each time user clicks a point.
- Insert box of type =[Replicator](/doc#box/Replicator) and name it **replicator**. It has to be between **solverParam** and **isSplit** on one side, and **solver**
  on the other side. Replicator has two input ports, **control_in** and **value_in**, and two output ports, **control_out** and **value_out**. Replicator will
  activate both output ports each time a new data frame arrives at **control_in** (processing will be cancelled if there is no data at **value_in** yet).
  The new data frame arrived to **control_in** will be passed to **control_out**, and then the last frame arrived to **value_in** will be sent to **value_out**.
  In our case, **control_in** is for solver parameters, and **value_in** is for initial point (it will be replicated to generate two parts of phase curve
  with different solver parameters).
- Edit connections between box ports:
    - remove isSplit:out_1 &mdash; solver:initState;
    - remove isSplit:out_2 &mdash; clickedPoints:input;
    - remove isSplit:out_3 &mdash; clickedPoints:flush;
    - remove solverParam:output &mdash; solver:parameters;
    - add isSplit:out_1 &mdash; replicator:value_in (here it's important to feed replicator:value_in before replicator:control_in);
    - add isSplit:out_2 &mdash; solverParam:activator (then solverParam will regenerate its output data);
    - add isSplit:out_3 &mdash; clickedPoints:input;
    - add isSplit:out_4 &mdash; clickedPoints:flush;
    - add solverParam:output &mdash; replicator:control_in;
    - add replicator:control_out &mdash; solver:parameters (we will set solver parameters <i>before</i> starting solver);
    - add replicator:value_out &mdash; solver:initState (we will start solver each time its parameters are changed);
- If necessary, position box ports by dragging them with the ```Ctrl``` key pressed down.
- Specify the **data** parameter of **solverParam** box. We need two array elements, so you will need to click the ```+``` button next to parameter name twice.
  Then enter the following parameters:
    - **data[0].h** = h;
    - **data[0].n** = 100000;
    - **data[0].nout** = 1;
    - **data[1].h** = -h;
    - **data[1].n** = 100000;
    - **data[1].nout** = 1;

Now the simulation should do what we want. One thing you might notice is that the resulting bitmap gets updated twice per point &mdash; once
for each part of the phase curve. To fix that, let's insert a counted filter between ports solver:finish and canvas:flush. If we set filter counter to 2,
it will pass each second signal, so we'll get what we want: one bitmap update per initial point.

So let's now do it:
- add a box of type =[CountedFilter](/doc#box/CountedFilter) and name it **flushFilter**;
- add a box of type =[Param](/doc#box/Param) and name it **flushFilterParam** (it will hold the filter count parameter);
- add connection flushFilterParam:output &mdash; flushFilter:count;
- set the **count** parameter of the **flushFilterParam** box to 2;
- remove connection solver:finish &mdash; canvas:flush (we need to filter data frames there);
- add connection solver:finish &mdash; flushFilter:input;
- add connection flushFilter:output &mdash; canvas:flush.

We are done! Now the output bitmap will update once per point clicked.

## Step 6. Marking phase curve with different colors
TODO

