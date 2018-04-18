# project3-viewshed-parallel-ethan-jake

Uses parallel code to compute a viewshed from a specified point in a .asc file. Prints
viewshed to a file.

The output grid corresponds exactly to the .asc grid -- a 1 signifies
that the point is visible from the viewpoint, a 0 means it is not
visible. Points labeled with a NODATA value in the .asc file will be
given the same NODATA value in the viewshed grid file.

We decided to take a conservative approach with NODATA visibility: If
a line of sight intercepts a NODATA point, no points behind the NODATA
point is visible.

After making the project, run the code with the following command:

./vs <input .asc file location> <output file location> <viewpoint row
cooridnate> <viewpoint column coordinate> <number of threads>

example:

./vs ./set1vis.100.100.asc output.asc 100 100 2
./vs ./set1vis.250.250.asc output.asc 250 250 8
