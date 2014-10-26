### Overview

This box provides =[interactive user input](/doc#page/general-interactive-input) in the form of entering numerical values.
It is one of _vector data input boxes_.

The box allows user to set arbitrary input values as numbers.
Input values, as well as their indices in output data frame, are specified in the **param** parameter.

The **param** parameter is an array of elements, each of which is a structure with the following fields:
* **index** &mdash; the index of component of data frame (received at port **input**) to be replaced with the value (to prepare data frame for port **output**);
* **name** &mdash; name to display near the slider corresponding to the value;

When user edits the $i$-th value and presses ```Enter```, he/she sets the value $v_i$.

Input data frames coming to port **input** are expected to be one-dimensional arrays of $n$ elements, and output data frames sent to port **output** have the same format.
Output data frames ${\bf x}^{out}=[x^{out}_0,\ldots,x^{out}_{n-1}]^T$ are obtained from input data frames ${\bf x}^{in}=[x^{in}_0,\ldots,x^{in}_{n-1}]^T$ as follows.
If there is no user input yet, then ${\bf x}^{out}={\bf x}^{in}$. Otherwise, some components of input
are replaced with the values entered:
$$
x^{out}_k=\left[\begin{array}{rl}
v_i,&\mbox{if }i\mbox{ is foud such that }k=k_i\mbox{ and value }v_i\mbox{ has been entered},\\
x^{in}_k&\mbox{otherwise}.
\end{array}\right.
$$
The indices $k_i$ are the values of parameters **param[i].index**.

The data processing for this box, as well as parameters **restartOnInput** and **activateBeforeRestart**, are described in the
**Data procesing** and **Vector data input** sections of the =[Interactive input](/doc#page/general-interactive-input) page.

### See also

* The =[RangeInput](/doc#box/RangeInput) box allows user to input values within specified ranges, with specified resolution.
