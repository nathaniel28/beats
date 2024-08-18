CFLAGS=-g -Wall -Wextra -pedantic -std=c++20 #-O2
LIBS=-lSDL2 -lGLEW -lGL -lvlc
CC=g++

beats: main.cpp shaders/sources.h argparse.o
	$(CC) $(CFLAGS) $(LIBS) argparse.o main.cpp -o beats

SHADER_INPUTS=shaders/note.vert shaders/note.frag shaders/highlight.vert shaders/highlight.frag
shaders.h: $(SHADER_INPUTS)
	./process_shader.py $(SHADER_INPUTS)

argparse.o: argparse/argparse.c argparse/argparse.h
	$(CC) $(CFLAGS) -c argparse/argparse.c

argparse.win.o: argparse/argparse.c argparse/argparse.h
	x86_64-w64-mingw32-g++ $(CFLAGS) -c argparse/argparse.c -o argparse.win.o

clean:
	rm -f beats
	rm -f shaders/sources.h
	rm -rf winbuild/*
	rm -f *.o

all:
	make clean
	make shaders.h
	make beats

windows: main.cpp shaders/sources.h argparse.win.o
	x86_64-w64-mingw32-g++ $(CFLAGS) -I./windepend/include -L./windepend/lib -lSDL2 -lglew32 -lopengl32 -llibvlc argparse.win.o main.cpp -o winbuild/beats.exe
