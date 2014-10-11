The box interprets data frames coming to the **input** port as sequential points of a curve. It interpolates the curve linearly between neighboring points.
Points of intersection with a hyperplane are computed. These points, and nothing else, are sent to the **output** port, provided that the crossing direction
is matched.

The hyperplane is determined by equation $x_k=c$, where
- $x_0, x_2,\ldots x_{n-1}$ are coordinates in space (space dimension $n$ is determined by data frame size at the **input** port);
- $k \in[0,n-1]$ is the index determining the hyperplane orientation (it is the value of **param.index** parameter);
- $c$ is the offset of the hyperplane from coordinate system origin (it is the value of **param.pos** parameter).

Plane crossing direction can occur in positive direction ($x_k$ increases) or negative direction ($x_k$ decreases). The box has a parameter, **param.flags**, that
specifies acceptable crossing directions. That parameter is an array containing one or two string values, ```'negative'``` and ```'positive'```. For example,
if the ```'negative'``` value is fond in the array, and the crossing direction is negative, the intersection point comes to **output**. Otherwise,
if the ```'negative'``` value is not fond in the array, and the crossing direction is still negative, the intersection point is skipped.
