CFLAGS=-g -Wall -Wextra -pedantic -std=c++20
LIBS=-lSDL2 -lSDL2_image
CC=g++

main: main.cpp
	$(CC) $(CFLAGS) $(LIBS) -o main main.cpp
