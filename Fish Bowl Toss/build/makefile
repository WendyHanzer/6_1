# Linux
CC=g++ 
LIBS= -lglut -lGLEW -lGL -lassimp -I/usr/include/bullet -lBulletSoftBody -lBulletDynamics -lBulletCollision -lLinearMath `Magick++-config --cppflags --cxxflags --ldflags --libs` -I"../src/include" -L"/usr/lib" -pthread


# Mac
##CC=clang++
##LIBS= -framework OpenGL -framework GLUT -framework Cocoa -lGLEW -stdlib=libc++ -lassimp -I/usr/local/include/bullet -lBulletSoftBody -lBulletDynamics -lBulletCollision -lLinearMath `Magick++-config --cppflags --cxxflags --ldflags --libs` -L/usr/local/lib/ -I/usr/local/include/ -I"../src/include" -L"/usr/lib" -pthread

# Compiler flags
CXXFLAGS= -g -Wall -std=c++0x 
all: fishbowl

main.o: ../src/main.cpp
	$(CC) $(CXXFLAGS) -c ../src/main.cpp -o ../src/main.o $(LIBS)

shaderLoader.o: ../src/shaderLoader.cpp
	$(CC) $(CXXFLAGS) -c ../src/shaderLoader.cpp -o ../src/shaderLoader.o $(LIBS)

mesh.o: ../src/mesh.cpp
	$(CC) $(CXXFLAGS) -c ../src/mesh.cpp -o ../src/mesh.o $(LIBS)

texture.o: ../src/texture.cpp
	$(CC) $(CXXFLAGS) -c ../src/texture.cpp -o ../src/texture.o $(LIBS)

camera.o: ../src/camera.cpp
	$(CC) $(CXXFLAGS) -c ../src/camera.cpp -o ../src/camera.o $(LIBS)

wind.o: ../src/wind.cpp
	$(CC) $(CXXFLAGS) -c ../src/wind.cpp -o ../src/wind.o $(LIBS)

fishbowl: main.o shaderLoader.o mesh.o texture.o camera.o wind.o
	$(CC) $(CXXFLAGS) -o ../bin/fishbowl ../src/main.o ../src/shaderLoader.o ../src/mesh.o ../src/texture.o ../src/camera.o ../src/wind.o $(LIBS)  ../bin/macosx-gcc/libirrklang.dylib 

clean:
	rm ../src/*.o ../bin/fishbowl
