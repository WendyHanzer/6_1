# Linux
CC=g++
LIBS= -lglut -lGLEW -lGL


# For Macs uncomment the next line and comment out the previous one
#CC=clang++
#LIBS=  -framework OpenGL -framework GLUT -framework Cocoa -lGLEW -stdlib=libc++
# Assuming you want to use a recent compiler

#Paths for my Mac
PATHI= -I/usr/local/include/ 
PATHL= -L/usr/local/lib/

# Compiler flags
CXXFLAGS= -g -Wall -std=c++0x

all: main.o shader.o object.o
	$(CC) $(CXXFLAGS) -o ../bin/table ../bin/main.o ../bin/shader.o ../bin/object.o $(LIBS) $(PATHI) $(PATHL)

main.o: ../src/main.cpp
	$(CC) $(CXXFLAGS) -c ../src/main.cpp -o ../bin/main.o $(PATHI) 

shader.o: ../src/shader.cpp ../src/shader.h
	$(CC) $(CXXFLAGS) -c ../src/shader.cpp -o ../bin/shader.o 

object.o: ../src/object.cpp ../src/object.h
	$(CC) $(CXXFLAGS) -c ../src/object.cpp -o ../bin/object.o $(PATHI)

clean:
	rm ../bin/*.o ../bin/table 
