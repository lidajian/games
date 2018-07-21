CC=g++
FLAGS=-std=c++11
M=20
N=20
all: snake
snake: main.cpp control_source.hpp game_board.hpp
	$(CC) $(FLAGS) main.cpp -o snake
run: snake
	./snake $(M) $(N)
clean:
	rm snake
