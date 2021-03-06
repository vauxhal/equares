# Simulation workflow
This section explains how simulation runs, i.e., how it processes data frames generated by boxes.

Further the main routine that controls the data processing is called _runner_.

## Activation notifications

When a box generates a data frame and sends it to an output port, it actually does two things:
* makes the new data frame available in its output port;
* _activates_ all links connected to the output port. This step can also be called _output port activation_.

Each link connects an output port to an input port, and its activation means sending notification to input port owner box.
The notification just says that a new data frame is available at that input port.

When a box receives such a notification, it is free to do whatever it wants to. In some cases, these notifications are ignored;
in other cases, they cause box to start processing data and generate output data frames, which leads to link activation again,
and the data processing goes one level deeper.
For example, the =[Pendulum](/doc#box/Pendulum) box has two input ports, **parameters** and **state**. When a data frame comes
to **parameters**, the activation notification is ignored (but next time the box will be able to read parameters from that port).
When a data frame comes to **state**, the activation is not ignored. Instead, the box computes ODE right hand side and sends it
to the output port **oderhs**.

## Cancellation of data processing

Link activation notification is actually a function call, and the box notified can return a value to the caller that indicates
success or failure. If link activation fails, the data processing is _cancelled_. This actually happens because some box cannot
obtain all data it needs from input ports. For example, the =[Pendulum](/doc#box/Pendulum) box can process the activation of link
connected to port **state** only if there are some parameters available in port **parameters**. If it is so, the activation
succeeds. Otherwise, the activation fails, and the processing is cancelled.

If a box sends a data frame to its output port, and the activation of that output port fails, the box always cancels
the data processing. Notice that this is always done by returning a value indicating link activation failure, because the
box can only do something within an activation notification.

## Data source box activation

Each simulation must have at least one _data source_ box &mdash; a box having output ports but no input ports
(see =[Boxes, ports, and links](/doc#page/general-items)). There could be more than one data source.

Data sources can be _passive sources_ or _generators_. A generator is a data source that can be notified just as a link can be.
A passive data source cannot be notified.

A passive data source produces one data frame (per output port) during the entire simulation.
The data frame is available on its output port from the very beginning of the simulation.

## Initialization of the queue of notifications

When the runner starts data processing, it first considers all data sources and builds the initlal state of the _queue of notifications_.
For each generator, its notification is enqueued. For each passive data source, the notification of each of its links is enqueued.

## Processing of the queue of notifications

Then the queue is processed by sending the activation notifications (i.e., calling notification functions) one by one,
from the beginning to the end. If a notification call succeeds, the notification is removed from the queue. Otherwise,
if the notification call fails (i.e., the data processing gets cancelled),
the notification is moved to the end of the queue, and the process continues.

The runner processes its queue of notifications until it becomes empty, or maximum number of activation notification failures
(currently 100) is exceeded. In the latter case, the entire simulation fails.

## Postprocessing

When the queue of notifications becomes empty, the runner can enqueue _postprocessors_ before it stops the data processing.
The only example of a postprocessor is the =[Pause](/doc#box/Pause) box. Postprocessors, like generators, are boxes that
can receive activation notifications.

## User input events

The above process normally takes place during the simulation. In addition, there could be events that break the processing
of the queue of notifications. These events are caused by =[interactive user input](/doc#page/general-interactive-input).
Once a user input event occurs, an exception is thrown, which leads to the unwinding of any nested link activation calls and
the change of the queue of notifications. Besides, each box gets notified about simulation restart.

The queue of notifications is changed as follows when user input occurs. First, the queue is cleared. Then one of two things
happens.
* If the box that threw the exception specifies which box should be activated after restart,
  the notifications for that box are enqueued (if the box is a generator, its activation notification is enqueued; otherwise,
  the activation notifications of all links connected to its output ports are enqueued). An input box can only specify
  itself as the next box to activate, or specify nothing.
* If the box that threw the exception specifies no box to be activated after restart,
  the standard initialization of the notification queue is done.

After that, the processing of notification queue continues.

### Notes
* The **restartOnInput** parameter of many =[input boxes](/doc#page/general-interactive-input) controls wheter box will specify itself as
  the box to notify after input event (```false```) or specify nothing (```true```).
* The **activateBeforeRestart** parameter of many input boxes control whether to perform output port activation before throwing
  input event exception (```true```) , or just to make new data frame available in the output port (```false```) .
