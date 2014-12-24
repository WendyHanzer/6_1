#OpenGL Air Hockey: CS 680 Graphics Game

###Group Members

- Andy Garcia
- Hardy Thrower
- Nolan Burfield

##Overview

The game will start off with a view of both players in the game and the puck in the center of the board, for best results make the game into a full screen mode. Initially the game will have an AI running the right side, and the player will control the left paddle with keys or mouse. The goal is to get the puck into the opposite side goal and the score will update on the top score board according to the team that got the goal. There will be the option to right click and select different models, and other game functionality. Keys can also be used to move the camera around, reset the puck, and reset the camera to the main view.

##System Requirement

- GLUT and GLEW
- glm
- Assimp
- ImageMagick
- Bullet Physics Library

##User Manual

###Compile and Run Program
	>$ cd build/
	>$ make
	>$ cd ../bin/
	>$ ./airhockey

###Game Play
######Keyboard Controls
- X-Camera (q, w)
- Y-Camera (e, r)
- Z-Camera (t, y)
- X-Focus (a, s)
- Y-Focus (d, f)
- Z-Focus (g, h)

- p will reset the puck
- SPACE will reset the camera to default position

######Paddle Movement
- Left Arrow, Right Arrow, Up Arrow, Down Arrow moves the left paddle accordingly.
- ‘,’ ‘/’ ‘l’ ‘.’ moves the right paddle accordingly to their location on the keyboard. 
- The mouse will move the right paddle towards the location of the movement.

######Mouse Right Click
- Start Game
- Pause Game
- End Game
- Restart Game
- Switch Paddles
- Switch Pucks
- Switch AI / Two Players


##Tech Manual

The game board renders with multiple triangles on the surface, and they create an unsmooth surface to glide on so the puck sometimes appears to get stuck or bounce around the lines. 

There is an issue with the puck that sometimes gets stuck under the surface of the board with the added friction, and so it falls through but they reset.

The movement of the paddles is also a little jittery if too much force is given to it.

