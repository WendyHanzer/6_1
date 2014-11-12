#OpenGL Simple Lighting

###Group Members

Nolan Burfield  
Hardy Thrower  
Andy Garcia  

##Overview

This is showing off the lighting in OpenGl. The menu can switch the shaders, and change objects. 
To add more shaders build them and put them in the shaders folder, then add a new menu in the shaders
submenu. Also in the menu function add in the load shader case for that new shader. 

##Compile and Run
Makefile is built for a Linux system.  
  
	cd build
	make
	./lighting

##Directions

###Menu
* Quit
* Shader Select
	* No Lighting Shader 
	* Vertex Light Shader
	* Fragment Light Shader
* Object Select
	* Wood Box
	* Earth Object

###Camera
- X-Camera (q, w)
- Y-Camera (e, r)
- Z-Camera (t, y)
- X-Focus (a, s)
- Y-Focus (d, f)
- Z-Focus (g, h)
