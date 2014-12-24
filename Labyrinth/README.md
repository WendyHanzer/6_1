#OpenGL Labryinth Game: CS 680 Graphics Game

###Group Members

- Andy Garcia
- Hardy Thrower
- Nolan Burfield

##Overview

The game starts off with the board oriented the goal at the back left of the board, and the ball at the back right of the board. To win the player uses the arrow keys to control the direction of the board which will move the ball around. Avoid the holes, and try to get the ball to the checkered hole. Menu options can be seen by right clicking. There will be the option to add more balls, or change to a different level. The scoring of the game is done based off the length of seconds it takes to get to the end. The display score is the previous score achieved, and the current time of the running game. 

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
	>$ ./labyrinth

###Game Play
######Board Controls
- Arrow Keys- Rotate Board
- Mouse - Rotate Board

######Camera Controls
- X-Camera (q, w)
- Y-Camera (e, r)
- Z-Camera (t, y)
- X-Focus (a, s)
- Y-Focus (d, f)
- Z-Focus (g, h)
- Space, move the camera to a different view

######Option Controls
- 'p' - reset the ball or all the balls to their beginning location.
- 'c' - toggle the spotlight on/off.
- 'v' - toggle the ambient on/off.
- 'b' - toggle the point light on/off.
- '=' - add more balls.
- '-' - remove balls.
- 'F1' -Toggle Mouse Visibility

######Mouse Right Click
- Start Game
- Pause Game
- End Game
- Restart Game
* Levels
  * Level 1
  * Level 2
* Balls
  * Add One More
  * Remove One
* Difficulties
  * Easy
  * Medium
  * Hard

##Tech Manual

Known Issues:

The current movement of the board is not as smooth as it should be. There is a lot of bounding checking that keeps the board in range, and that would be optimized if given more time. On harder difficulties sometimes the ball bounces out of the board.
