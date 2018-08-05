CC=g++
FLAGS=-std=c++11 -I../include/
all: chinese_checker
chinese_checker: chinese_checker.cpp ../include/*.hpp
	$(CC) $(FLAGS) chinese_checker.cpp -o chinese_checker
run: chinese_checker
	./chinese_checker
install: chinese_checker
	cp chinese_checker /usr/local/bin
uninstall:
	rm -f /usr/local/bin/chinese_checker
clean:
	rm -f chinese_checker
