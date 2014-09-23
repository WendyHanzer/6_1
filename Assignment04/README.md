This is a simple Model Loader Program
========================================

Commands
--------

There will be a model that is specified to be loaded on the screen rotating.
There option to zoom in and out is handled by '=' '-' keys.
The command line is given the shaders, and object file name. 
The object is colored if there is a material file with the 
same name as the obj, located in the obj directory, and is a .mtl file.

Directions:

Zoom In: '='

Zoom Out: '-'

Escape Key quits game.

Building This Example
---------------------

*This example requires GLM*

*To Make The Program*

Navigate to the build directory.

>$ cd build

>$ make

*If you are using a Mac you will need to edit the makefile in the build directory*

The excutable will be put in bin

To run the program

*Program takes shader file and object names as command line arguments in the order of: vertexShader fragmentShader objectFile*

>$ cd ../bin

>$ ./table vertexShader.glsl fragmentShader.glsl table.obj

