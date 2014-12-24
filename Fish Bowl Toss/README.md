#OpenGL Fish Bowl Carnival Style Game

###Group Members

- Andy Garcia
- Hardy Thrower
- Nolan Burfield

##Overview

This game goal is to toss a gold ball into one of the fish bowls. Doing so will 
give an instant replay of the shot, and then the bowl explodes. This OpenGL game
has a lot involved in it; Sound, Physics, Textures, and User Input.

##System Requirement

- GLUT and GLEW
- glm
- Assimp
- ImageMagick
- Bullet Physics Library
- irrKlang (included in git)

##User Manual

###Compile and Run Program
	>$ cd build/
	>$ make
	>$ ./../bin/fishbowl

###Game Play
######Keyboard Controls
- X-Camera (q, w)
- Y-Camera (e, r)
- Z-Camera (t, y)
- X-Focus (a, s)
- Y-Focus (d, f)
- Z-Focus (g, h)

- p will reset the game
- SPACE will reset the camera to default positions

######Ball Toss
- '=' increase the toss power of the ball
- '-' decrease the toss power of the ball
- Click the screen to throw the ball, position is taken into account for the throw.


######Mouse Right Click
- End Game
- Restart Game

