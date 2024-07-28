CFLAGS=-g -Wall -Wextra -pedantic -std=c++20
LIBS=-lSDL2 -lSDL2_image -lSDL2_mixer
CC=g++

beats: main.cpp
	$(CC) $(CFLAGS) $(LIBS) -o beats main.cpp
