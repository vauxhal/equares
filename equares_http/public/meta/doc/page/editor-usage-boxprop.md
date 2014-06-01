# Editing box parameters

=[Boxes](/doc#page/general-items) have **parameters**. Each box has at least these two parameters:
* **name** is what is displayed on the box in the =[simulation scheme](/doc#page/editorpane-scheme);
* **type** is the type of the box (for a list of all box types, go to the **Boxes** section of the documentation).

Actually each box has more parameters; the list of these parameters depends on box type.
To edit parameters of a box, click the box on the scheme. The box will be selected,
and the =[object parameters pane](/doc#page/editorpane-prop) will display its parameters.
The parameters are edited right there, or (if a parameter has multiple lines of text) in the =[text editor](/doc#page/editorpane-text).

![](/meta/doc/page/editor-usage-boxprop-1.png 'Parameters of the selected box')

**Note**
* Renaming a box is changing its **name** parameter. It's important that two boxes in the simulation
  cannot have the same name. That's why the editor will not allow you to change box name if the new name
  coincides with the name of some other box.
* When you change the **type** parameter of a box, side effects may take place. Make sure to read about =[changing box type](/doc#page/editor-usage-replacebox).
