The box serves as data storage &mdash; it has no output ports.
It receives data frames of any format in port **input** and writes them to the output text file. Each data frame becomes a single line of the output file.

Internal file buffer is flushed and actually written to the disk as soon as any data frame comes to port **flush**.
Notice that flushing the buffer frequently (e.g., after each data frame) can potentially slow down simulation.

The name of the output file is specified by the **fileName** parameter. The file name must contain no path, so that the file is created in user's
local directory. The name must also end with the ```.txt``` filename extension. If these conditions are not met, the simulation is cancelled.

For technical reasons, there is a limitation on the size of the output file. Currently the total number of numerical values written to the file cannot exceed
$10^6$. Once the limit is reached, the file is closed and no more output occurs.
