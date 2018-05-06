CC=g++
CFLAGS = -g -Wall
VPATH = .:src:include
HEADER=-I./include

objects = main.o command.o
main: $(objects)
	$(CC) -o main $(objects)
main.o: main.cpp command.h
	$(CC) -o main.o $(CFLAGS) $(HEADER) -c main.cpp
command.o: command.cpp command.h
	$(CC) -o command.o $(CFLAGS) $(HEADER) -c ./src/command.cpp

.PHONY : clean
clean:
	rm main $(objects)