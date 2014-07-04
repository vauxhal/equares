# Editing info text

Simulation has the **info** =[property](/doc#page/editor-usage-simprop). It should contain an extended description of the simulation.
Simulatoin description is formatted and displayed in the =[info pane](/doc#page/editorpane-info).

To edit the property, click the **...** button next to its name in the [object parameters pane](/doc#page/editorpane-prop) and modify text in the [text editor](/doc#page/editorpane-text).

It's good to keep in mind the following things about the **info** property.
1. You can use the ?[markdown](http://daringfireball.net/projects/markdown/) format, or, more specifically,
   its ?[gfm](https://help.github.com/articles/github-flavored-markdown) flavor. In addition, we allow adding the **?** prefix to a link
   for making it open in a new tab.
2. You can use ?[TeX](http://tug.org/) formulas. In-line formulas should be enclosed in dollar signs (\$), and
   displayed formulas should be enclosed in double-dollar sequences (\$\$). This makes entering formulas like $$\ddot\varphi+\sin\varphi=0$$ quite an easy job &mdash;
   just type<br/> \$\$```\ddot\varphi+\sin\varphi=0```\$\$.
3. For hyperlinks, use this syntax: ```?[label](url)```. The **?** prefix will force the link to open in a new browser tab, which seems to be appropriate in the =[editor](/editor) in most cases.
   To open link in the same browser tab, use either ```=[label](url)``` or ```[label](url)```.
4. For images, use this syntax: ```![alt](url)``` or ```![alt](url "title")```. You can upload images to our server &mdash; see =[Adding images](/doc#page/editor-usage-images).
5. If you show the info pane when editing the **info** property, you will see it formatted immediately as you type.
6. Don't forget to click the **Ok** button when finished!
