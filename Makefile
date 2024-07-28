CFLAGS=-g -Wall -Wextra -pedantic -std=c++20 -O2
LIBS=-lSDL2 -lSDL2_image -lSDL2_mixer
CC=g++

main: main.cpp
	$(CC) $(CFLAGS) $(LIBS) -o main main.cpp
