### Overview
This box produces a grid of values for each input data frame.
Input data frames must be one-dimensional arrays.

For each input data frame, the grid generator produces, $N$ output frames,

$$
  N = N_0 N_1\ldots N_{d-1},
$$

$d$ is the number of grid dimensions, and $N_k$ is the grid size in $k$-th dimension, $0\le k \lt d$.

The grid consists of points ${\bf x}_I$. Each point is identified by multi-index $I=i_1,i_2,\ldots,i_d$
(indices $i_k$ run from 0 to $N_k-1$) and has coordinates $x^1_K, x^2_K, \ldots, x^d_K$.

Coordinates are computed as follows:

$$
x^k_K = (1-t) x^{k,\min} + t x^{k,\max}, \quad t = \frac{i_k}{N_k-1}.
$$

Therefore, $k$-th dimension of the grid is described by parameters $N_k$, $x^{k,\min}$, and $x^{k,\max}$.
Each coordinate of each grid point replaces some component of input data frame. Dimension $k$ is tied to the
index of component of input data frame to replace, so that index is one more grid dimension parameter.

Output data frames are same as the input one, except that $d$ components are replaced by grid point coordinates $x^k_K$,
for all values of multi-index $K$. As soon as there are $N$ points in the grid, $N$ output frames are produced per one input frame.

The grid is defined by elements of the **param** parameter, which is an array.
The length of the array is the number of grid dimensions, $d$.
Each $k$-th element of the array is a structure defining the $k$-th grid dimension. It has the following fields:
* **index** &mdash; index of element in the input frame to substitute with $k$-th coordinates of grid points, $x^k_K$.
* **vmin** &mdash; minimum grid value, $x^{k,\min}$.
* **vmax** &mdash; maximum grid value, $x^{k,\max}$.
* **count** &mdash; number of grid values, $N_k$.

### Data processing

Once a data frame comes to the **input** port, the following happens.
* $N$ frames are written to the **output** port &mdash; see overview above.
* After that, an empty data frame is sent to the **flush** port.

If a data frame comes to the **range** port, its data defines
new values of $x^{k,\min}$ and $x^{k,\max}$, $k=1,\ldots,n$, and $n$ is the half (rounded down) of the number of elements
in the data frame.
This feature is typically used in combination with the =[RectInput](/doc#box/RectInput) box that supplies
new ranges as user pans/zoom output image.

### See also

The following simulations show examples of usage for this box.
* =[Ince-Strutt diagram](/editor?sim=mathieu-stability) &mdash; basic usage example;
* =[Mandelbrot set](/editor?sim=mandelbrot) and =[Mandelbrot set (enhanced, color-1)](/editor?sim=mandelbrot-enhanced-color-1) &mdash;
  a little bit more advanced examples, in which the **range** input port is employed.
