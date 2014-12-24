Model Loader with Textures
========================================

Commands
--------

There will be a model that is specified to be loaded on the screen rotating.
There option to zoom in and out is handled by '=' '-' keys.
The command line is given the shaders, and object file name. 
The object is colored if there is a material file with the 
same name as the obj, located in the obj directory, and is a .mtl file.
This will load a single texture into the buffer, and place that single 
texture on the object loaded.

Directions:

Zoom In: '='
Zoom Out: '-'

Escape Key quits game.

Building This Example
---------------------

*This example requires GLM, and uses ImageMagick for Image loading*

*To Make The Program*

*This Program was built and Run on a Mac, but should transition to Ubuntu Fine*

Navigate to the build directory.

>$ cd build

>$ make

*If you are using a Mac you will need to edit the makefile in the build directory*

The excutable will be put in bin

To run the program with textured bix built in:

>$ cd ../bin

>$ ./table

The program also can take a texture as a command line so to run that:

*Program takes shader file and object names as command line arguments in the order of: vertexShader fragmentShader objectFile*

>$ cd ../bin

>$ ./table vertexShader.glsl fragmentShader.glsl <Object>
