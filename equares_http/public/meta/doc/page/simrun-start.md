# Running simulation

Our server only allows running simulations for users who are logged in.

To run a simulation, load it in the =[editor](/editor) and select the **Run** command in the editor menu.

![simrun menu](/meta/doc/page/simrun-start-1.png "Menu command to run simulation")

As you do that, the running simulation dialog pops up.

![simrun dlg](/meta/doc/page/simrun-start-2.png "Running simulation dialog")

The dialog normally contains running status information (running or finished), followed by =[simulation output](/doc#page/simrun-output).

In case of an error, the simulation dialog will contain error messages from NumEquaRes backend.

![simrun dlg error](/meta/doc/page/simrun-start-3.png "Running simulation error")

To terminate a running simulation, click the **Stop** button in the dialog. The simulation will stop, but the dialog window will remain.
The **Stop** button is unavailable if simulation is no longer running.

To close the running simulation dialog, click the **Close** button or use the close icon in the upper right corner of the dialog. When the dialog
is closing, it stops simulation in case it is still running.
