# Changing box type

Box type is determined by its **type** [parameter](/doc#page/editor-usage-boxprop).
You can change the value of the parameter by selecting some other type in the drop-down list next to property name in the [object parameters pane](/doc#page/editorpane-prop).


Notice that changing box type is actually replacing the object with a new one. Therefore, some side effects typically occur when you replace box type.
* If the old box had connections on its ports, the editor tries to re-create the same links on the replaced box. However, it might happen that
  some links will be lost. As long as changing box type can result in a completely different box, port formats and even the sets of input and
  output ports can change. If there is no match between ports of the new box and connected ports of the old box, old links will be lost.
* It can happen that user have specified parameters determined by the replaced box in some other box. For example, there could be a
  =[Pendulum](/doc#box/Pendulum) box (named **ode**) having its **parameters** port connected by a link with the **output** port of a [Param](/doc#box/Param) box (named **odeParam**).<br/>
  ![before changing type](/meta/doc/page/editor-usage-replacebox-1.png "ODE parameters before changing box type")<br/>
  Now suppose you change the type of **ode** from Pendulum to =[DoublePendulum](/doc#box/DoublePendulum).<br/>
  ![changing box type](/meta/doc/page/editor-usage-replacebox-2.png "Changing box type")<br/>
  At this point, editor removes the old **Pendulum** box (as well as its link to **odeParam**) and creates the new one of type **DoublePendulum** instead of it. The editor then re-creates the link
  to **odeParam**. Due to the re-creation of the link, the data stored in **odeParam** is lost. And, on the other hand, it's now different data anyway, since
  the format of the **parameters** input port of the **ode** box has changed.
  ![changing box type](/meta/doc/page/editor-usage-replacebox-3.png "Parameters after changing box type")<br/>
