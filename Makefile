CFLAGS=-g -Wall -Wextra -pedantic -std=c++20 #-O2
LIBS=-lSDL2 -lGLEW -lGL -lvlc
CC=g++
DEST=./beats

ifeq ($(PLATFORM), windows)
	CFLAGS+=-I./windepend/include -L./windepend/lib 
	LIBS=-lSDL2 -lglew32 -lopengl32 -llibvlc
	CC=x86_64-w64-mingw32-g++
	DEST=./winbuild/beats.exe
endif

OBJS=argparse.o chart.o util.o keys.o
beats: main.cpp shaders/sources.h $(OBJS)
	$(CC) $(CFLAGS) $(LIBS) $(OBJS) main.cpp -o $(DEST)

SHADERS=shaders/note.vert shaders/note.frag shaders/highlight.vert shaders/highlight.frag
shaders/sources.h: $(SHADERS)
	./process_shader.py $(SHADERS)

chart.o: chart.cpp chart.hpp
	$(CC) $(CFLAGS) -c chart.cpp

util.o: util.cpp util.hpp
	$(CC) $(CFLAGS) -c util.cpp

keys.o: keys.cpp keys.hpp
	$(CC) $(CFLAGS) -c keys.cpp

argparse.o: argparse/argparse.c argparse/argparse.h
	$(CC) $(CFLAGS) -c argparse/argparse.c

clean:
	rm -f shaders/sources.h
	rm -f *.o

all:
	make clean
	make shaders.h
	make beats
