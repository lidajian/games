CC=g++
HOST=localhost
PORT=8711
FLAGS=-std=c++11 -I../include/ -I/usr/local/Cellar/boost/1.67.0_1/include -lboost_thread-mt -lboost_system-mt
all: chinese_checker chinese_checker_server
chinese_checker: chinese_checker.cpp ../include/*.hpp
	$(CC) $(FLAGS) chinese_checker.cpp -o chinese_checker
chinese_checker_server: chinese_checker_server.cpp ../include/*.hpp
	$(CC) $(FLAGS) chinese_checker_server.cpp -o chinese_checker_server
run: chinese_checker
	./chinese_checker
run0: chinese_checker
	./chinese_checker $(HOST) $(PORT) 0
run1: chinese_checker
	./chinese_checker $(HOST) $(PORT) 1
runserver: chinese_checker_server
	./chinese_checker_server $(PORT)
install: chinese_checker
	cp chinese_checker /usr/local/bin
uninstall:
	rm -f /usr/local/bin/chinese_checker
clean:
	rm -f chinese_checker chinese_checker_server
