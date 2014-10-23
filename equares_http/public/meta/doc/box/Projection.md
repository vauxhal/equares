The box receives input data frames at port **input** and sends output data frames to port **output**.
Input port format is arbitrary, but it always is treated as one-dimensional array containing $n$ elements.
Output port format is a one-dimensional array of $m$ elements, where $m$ is the number of elements in the **indices** parameter.

The box produces one output data frame ${\bf x}^{out}=[x^{out}_0,\ldots,x^{out}_{m-1}]^T$ for each input data frame ${\bf x}^{in}=[x^{in}_0,\ldots,x^{in}_{n-1}]^T$.
The components of the output data frames make a subset of components of the input data frame:
$$
x^{out}_k = x^{in}_{i_k}, \quad k=0,\ldots m-1.
$$
Indices $i_k$ are elements of the **indices** parameter, which is an array of $m$ elements. When specifying this parameter,
user should provide a comma-separated list of integers, e.g., ```0,1,3```.

Boxes of type **Projection** are used in almost each simulation. For example, see the =[Simple pendulum, phase trajectory](/editor?sim=simple-pendulum-1) simulation.
