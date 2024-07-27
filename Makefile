CFLAGS=-g -Wall -Wextra -pedantic
LIBS=-lSDL2
CC=g++

main: main.cpp
	$(CC) $(CFLAGS) $(LIBS) -o main main.cpp
