# Digital Microfluidic Biochip Simulator

___Long assignment 1 for THU Programming Practices course, August 2019___

## Description
A simulator frontend for digital microfluidic biochips (DMFB). Enter the size of an electrode grid, specify input/output ports, load the command file, and the result is presented on the screen with animation effects.

## Environment
__Required__: Qt (5.13.0 or higher)

## Supported Commands
Values mentioned below are all integers.

These commands only guarantee that the droplets are placed at the expected positions **at the moments specified**, but not how they actually move.

* `Input t x y`: Put a new droplet on the grid at position (x, y) on moment t, where an input port is required;
* `Output t x y`: Remove the droplet at (x, y) on moment t. This position must be next to an output port;
* `Move t x1 y1 x2 y2`: Start moving the droplet from (x1, y1) to (x2, y2) on moment t, which takes 1 second. The two grids must be next to each other;
* `Mix t x0 y0 x1 y1 x2 y2 ... xn yn`: Start moving the droplet at (x0, y0) on moment t through (x1, y1), (x2, y2), ..., (xn, yn), which takes 1 second per step. Each step must travel exactly 1 grid;
* `Merge t x1 y1 x2 y2`: Start merging the two droplets at (x1, y1) ans (x2, y2) on moment t, which takes 2 seconds. Distance of the two grids must be exactly 2;
* `Split t x1 y1 x2 y2 x3 y3`: Start splitting the droplet at (x1, y1) on moment t to two new droplets, which are positioned at (x2, y2) and (x3, y3). The splitting process takes 2 seconds. (x2, y2), (x1, y1) and (x3, y3) must form a straight segment of three grids.

## Constraints
* __Static constraint__: Distance of any pair of droplets cannot be less than 2 at any time;

* __Dynamic constraint__: No matter how the droplets actually move, distance of any pair of droplets cannot be anyhow possibly less than 2 at any moment.
