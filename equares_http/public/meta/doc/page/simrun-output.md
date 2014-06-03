# Simulation output

Simulation output consist of files determined by =[boxes](/doc#page/general-items) that are **data storage**.
Currently data storage box types are the following:
* =[Bitmap](/doc#box/Bitmap) &mdash; produces an image file;
* =[Dump](/doc#box/Dump) &mdash; produces a text file shown as a table.

When simulation starts, it inspects boxes that produce output files. This way the backend knows what output files are,
and thus we can have these files directly in the =[running simulation](/doc#page/simrun-start) dialog.

![simulation output](/meta/doc/page/simrun-output-1.png "Simulation output")

**Note** that you should provide appropriate file names for output files such that the names are valid and any name conflicts are avoided.
See data storage box documentation for more information.
