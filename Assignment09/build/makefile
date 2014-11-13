# Linux
CC=g++ 
LIBS= -lglut -lGLEW -lGL -lassimp `Magick++-config --cppflags --cxxflags --ldflags --libs`

# Mac
#CC=clang++
#LIBS= -framework OpenGL -framework GLUT -framework Cocoa -lGLEW -stdlib=libc++ -lassimp `Magick++-config --cppflags --cxxflags --ldflags --libs`

# Compiler flags
CXXFLAGS= -g -Wall -std=c++0x

all: lighting

main.o: ../src/main.cpp
	$(CC) $(CXXFLAGS) -c ../src/main.cpp -o ../src/main.o $(LIBS)

shaderLoader.o: ../src/shaderLoader.cpp
	$(CC) $(CXXFLAGS) -c ../src/shaderLoader.cpp -o ../src/shaderLoader.o $(LIBS)

mesh.o: ../src/mesh.cpp
	$(CC) $(CXXFLAGS) -c ../src/mesh.cpp -o ../src/mesh.o $(LIBS)

texture.o: ../src/texture.cpp
	$(CC) $(CXXFLAGS) -c ../src/texture.cpp -o ../src/texture.o $(LIBS)

lighting: main.o shaderLoader.o mesh.o texture.o
	$(CC) $(CXXFLAGS) -o ../bin/lighting ../src/main.o ../src/shaderLoader.o ../src/mesh.o ../src/texture.o $(LIBS)

clean:
	rm ../src/*.o ../bin/lighting
