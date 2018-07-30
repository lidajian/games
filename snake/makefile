CC=g++
FLAGS=-std=c++11 -I../include/
M=20
N=20
all: snake
snake: snake.cpp ../include/*.hpp
	$(CC) $(FLAGS) snake.cpp -o snake
run: snake
	./snake $(M) $(N)
clean:
	rm snake
