# Simulation properties

Simulation paroperties are displayed in the =[object parameters pane](/doc#page/editorpane-prop)
when no =[box](/doc#page/general-items) is selected in the =[simulation scheme](/doc#page/editorpane-scheme).
Therefore, to display simulation properties click on an empty area in the scheme (this will unselect the currently selected box, if any).

Each simulation has the following properties.

* **name** Name of simulation. On our server, =[saved simulations](/doc#page/simfile-save) are identified by username and simulation name.
  Change this parameter to clone an existing simulation.
  Note that if you specify the name of another your existing simulation here,
  and then save new simulation, it will overwrite the old one (you will be warned about that).
* **description** Short one line textual description of simulation. Displayed in =[simulation table](/doc#page/simfile-table).
* **info** Extended multiline description of simulation. Can contain pictures. Uses the
  ?[gfm](https://help.github.com/articles/github-flavored-markdown) flavor of
  the ?[markdown](http://daringfireball.net/projects/markdown/) format. See =[Editing info text](/doc#page/editor-usage-text-info) for details.
* **keywords** A comma-separated list of keywords. Each keyword may be a single word or a list space separated words.
  Can be helpful in finding simulations (see =[simulation table](/doc#page/simfile-table)).
* **script** JavaScript code to be executed at start of simulation. Can be used to declare constant parameters.
  These parameters can be used in expressions that provide values for other parameters. This feature is employed in
  the =[vibrating pendulum](/editor?sim=vibrating-pendulum-psec) example simulation.
* **public** If true, the simulation is visible in the =[table of simulations](/doc#page/simfile-table) to all users
  when =[saved](/doc#page/simfile-save). Otherwise, only visible to its owner.
