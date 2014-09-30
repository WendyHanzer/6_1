This is a simple Model Loader Program Using Assimp
========================================

Commands
--------

There will be a model that is specified to be loaded on the screen rotating.
There option to zoom in and out is handled by '=' '-' keys.
This will load any file: tested with .3ds, .dae, .ply, .stl.
This runs on Assimp, and hopefully does not Seg fault this time.

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

