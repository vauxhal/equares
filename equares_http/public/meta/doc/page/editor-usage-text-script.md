# Editing source code

Source code can be a parameter of a =[box](/doc#page/general-items). For example, the =[CxxOde](/doc#box/CxxOde) box has the **src** parameter.
Its value is the source code describing ODE system in the C++ language.

To access the value of the source code property, it's necessary to select the box first, and then press the **...** button
right to the name of source code property. =[Text editor](/doc#page/editorpane-text) will then open. It will display the source code,
as well as line numbers:

![](/meta/doc/page/editor-usage-text-script-1.png 'Editing source code')

Once you save the value of source code property by pressing the **Ok** button, the code is passed to the server for compilation.
During the compilation, box status is set to **wait**: ![waiting](/images/status-waiting.png).
As soon as the compilation finishes, box status is set to either **ok** ![ok](/images/status-ok.png) or
**error** ![error](/images/status-error.png). In the latter case, compiler error messages can be viewed by hovering the the status icon:

![](/meta/doc/page/editor-usage-text-script-2.png 'Viewing compiler error messages')

User should then fix the error by editing the source code and press the **Ok** button again. Finally, box status should be **ok** ![ok](/images/status-ok.png) before running the simulation.
