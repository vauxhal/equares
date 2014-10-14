The box computes differences of input data frames (i.e., coming to port **input**) and sends these differences to the output (i.e., to port **output**):
$$
x^{out}_k = x^{in}_{k+1} - x^{in}_k,
$$
where $x^{in}_k$ is the $k$-th input data frame, and $x^{out}_k$ is the $k$-th output data frame.
Thus, from $n$ input data frames it produces $n-1$ output frames.

The format of data frames is arbitrary; it has to be the same for input and output.
