SOURCE = main.c
CC = gcc
FLAGS = -lpthread -lm -Wall
FILE_NAME = mandelbrot.out
DEPS = timer.h

all: $(FILE_NAME)
	
$(FILE_NAME): $(SOURCE)
	$(CC) $(SOURCE) $(DEPS) -o $(FILE_NAME) $(FLAGS)
