CC=g++
CFLAGS = -g -Wall
HEADER=-I./include

main: main.o command.o
	$(CC) -o main main.o command.o
main.o: main.cpp
	$(CC) -o main.o $(CFLAGS) $(HEADER) -c main.cpp
command.o: ./src/command.cpp
	$(CC) -o command.o $(CFLAGS) $(HEADER) -c ./src/command.cpp
clean:
	rm main *.o