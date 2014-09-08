A simple example of matrix use in OpenGL
========================================

Commands
--------

You should see a color cube in orbit around nothing in a vast blueish nothing.
The Color cube should be rotating on y-axis.

Directions:

Spin Direction Left: 'z'
Spin Direction Right: 'x'
Rotate Direction Left: ','
Rotate Direction Right: '.'

Left-Click To Reverse Rotation

Right-Click To Bring Up Menu

Building This Example
---------------------

*This example requires GLM*
*On ubuntu it can be installed with this command*

>$ sudo apt-get install libglm-dev

*On a Mac you can install GLM with this command(using homebrew)*
>$ brew install glm

To build this example just 

>$ cd build

>$ make

*If you are using a Mac you will need to edit the makefile in the build directory*

The excutable will be put in bin

To run the program

*Program takes shader file names as command line arguments in the order of: vertexShader fragmentShader*

>$ cd ../bin

>$ ./Matrix vertexShader.glsl fragmentShader.glsl

Additional Notes For OSX Users
------------------------------

Ensure that the latest version of the Developer Tools is installed.
