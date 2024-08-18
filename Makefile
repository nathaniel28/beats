CFLAGS=-g -Wall -Wextra -pedantic -std=c++20 #-O2
LIBS=-lSDL2 -lGLEW -lGL -lvlc
CC=g++

beats: main.cpp shaders/sources.h
	$(CC) $(CFLAGS) $(LIBS) -o beats main.cpp

SHADER_INPUTS=shaders/note.vert shaders/note.frag shaders/highlight.vert shaders/highlight.frag
shaders.h: $(SHADER_INPUTS)
	./process_shader.py $(SHADER_INPUTS)

clean:
	rm -f beats
	rm -f shaders/sources.h

all:
	make clean
	make shaders.h
	make beats

windows: main.cpp shaders/sources.h
	cp -r windepend/lib/* winbuild
	x86_64-w64-mingw32-g++ -std=c++20 -I./windepend/include -L./windepend/lib -lSDL2 -lglew32 -lopengl32 -llibvlc main.cpp -o winbuild/beats.exe
