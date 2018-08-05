CC=g++
FLAGS=-std=c++11 -I../include/
M=20
N=20
all: snake snakevim
snake: snake.cpp ../include/*.hpp
	$(CC) $(FLAGS) snake.cpp -o snake
snakevim: snake.cpp ../include/*.hpp
	$(CC) -DVIM $(FLAGS) snake.cpp -o snakevim
run: snake
	./snake $(M) $(N)
vim: snakevim
	./snakevim $(M) $(N)
install: snake snakevim
	cp snake /usr/local/bin
	cp snakevim /usr/local/bin
uninstall:
	rm -f /usr/local/bin/snake
	rm -f /usr/local/bin/snakevim
clean:
	rm -f snake snakevim
