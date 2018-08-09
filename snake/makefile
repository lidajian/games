CC=g++
FLAGS=-std=c++11 -I../include/ -I/usr/local/Cellar/boost/1.67.0_1/include -lboost_thread-mt -lboost_system-mt
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
