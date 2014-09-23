A simple example of matrix use in OpenGL
========================================

Commands
--------

There will be two cubes, one is earth the one is the moon, the moon is scaled to 27% of the earth.
The moon will rotate around the earth in the same direction the earht spins, changing rotation of the 
earth will not change the moon rotation, but the spinning of the earth will change the moon. 

Directions:

Spin Direction Counterclockwise: 'z' and Down Arrow

Spin Direction Clockwise: 'x' and Up Arrow

Rotate Direction Clockwise: ',' and Right Arrow

Rotate Direction Counterclockwise: '.' and Left Arrow

Left-Click To Reverse Rotation

Right-Click To Bring Up Menu

The default "Year" of the cube is 5 minutes. 
Press 1 to change the year to 1 minute.

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

