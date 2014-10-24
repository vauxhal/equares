### Overview

This box provides =[interactive user input](/doc#page/general-interactive-input) in the form of moving sliders.
It is one of _vector data input boxes_.

The box allows user to set input values in specified ranges, with specified resolution.
Input values, as well as their indices in output data frame, are specified in the **param** parameter.

The **param** parameter is an array of elements, each of whichis a structure with the following fields:
* **index** &mdash; the index of component of data frame (received at port **input**) to be replaced with the value (to prepare data frame for port **output**);
* **name** &mdash; name to display near the slider corresponding to the value;
* **vmin** &mdash; minimum allowed value;
* **vmax** &mdash; maximum allowed value;
* **resolution** &mdash; value resolution (determines how many intermediate values can be set between minimum and maximum).

When user moves a slider corresponding to the $i$-th value, he/she sets some parameter $t_i\in[0,1]$, and the value $v_i$ is then computed as follows.
$$
v_i = v_{i,\min}(1-t_i^{*}) + v_{i,\max}t_i^{*}, \quad t_i^{*} = \frac1{N_i} \left\lfloor t_i{N_i} \right\rfloor,
$$
where $N_i>0$ is the resolution, **param[i].resolution**, $v_{i,\min}$ is the minimum value, **param[i].vmin**, $v_{\max}$ is the maximum value, **param[i].vmax**,
and $i=0,\ldots,m-1$ is the index in the **param** array of $m$ elements.

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

* The =[SimpleInput](/doc#box/SimpleInput) box allows user to input arbitrary values.
* The following simulations show examples of usage for the **PointInput** box:
    * =[Double pendulum, Poincare map (interactive 2)](/editor?sim=z%2Fdouble-pendulum-psec-interactive2);
    * =[Julia set for quadratic polynomial](/editor?sim=julia).
