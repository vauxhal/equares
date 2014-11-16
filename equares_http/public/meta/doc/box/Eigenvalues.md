### Overview

The box computes eigenvalues of square matrices, as soon as they arrive to the **matrix** input port. For each input matrix (one data frame coming to the **matrix** port),
real and imaginary parts of all eigenvalues are sent to output ports **eig_real** and **eig_imag** respectively.

### Notes
This box is only available if ```equares_core``` is built with the AMD's ?[ACML](http://developer.amd.com/tools-and-sdks/cpu-development/amd-core-math-library-acml/)
library (an implementation of ?[LAPACK](http://www.netlib.org/lapack/)).
To enable the use of ACML, the ```ACML_DIR``` variable needs to be supplied to ```qmake``` when building ```equares_core```.
